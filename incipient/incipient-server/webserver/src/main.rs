#![feature(custom_derive, custom_attribute, plugin)]
#![plugin(diesel_codegen, dotenv_macros)]
#[macro_use] extern crate hyper;
#[macro_use] extern crate nickel;
#[macro_use] extern crate diesel;
extern crate dotenv;
extern crate csv;

pub mod schema;
pub mod model;

use nickel::{Nickel, HttpRouter, Action, NickelError};
use nickel::{MiddlewareResult, Response, Request};
use nickel::status::StatusCode;
use diesel::prelude::*;
use diesel::pg::PgConnection;
use dotenv::dotenv;
use std::env;

use model::*;
use schema::users;
use schema::doors;

pub fn establish_connection() -> PgConnection {
    dotenv().ok();

    let database_url = env::var("DATABASE_URL")
        .expect("DATABASE_URL must be set");
    PgConnection::establish(&database_url)
        .expect(&format!("Error connecting to {}", database_url))
}

/// A simple stateless way of doing basic error handling.
/// Unused parts are prefixed with '_'.
#[derive(PartialEq, Eq)]
enum Status {
	Ok,
	_InvalidMail,
	_UserNotExist,
	_UnknownError,
	InvalidCreds,
	DoorNotExist,
	_InvalidUser,
	_InsecurePassword
}


fn create_door<'a>(conn: &PgConnection, user: i64, password: &'a str) -> Door {
    let new = NewDoor {
        user_id: user,
        password: password
    };

    diesel::insert(&new).into(doors::table)
        .get_result(conn)
        .expect("Error saving new post")
}

fn create_user<'a>(conn: &PgConnection, user: &'a str, mail: &'a str, password: &'a str) -> User {
    let new = NewUser {
        username: user,
        mail: mail,
        password: password
    };

    diesel::insert(&new).into(users::table)
        .get_result(conn)
        .expect("Error saving new post")
}



impl Door {
	/// Connect to the door (return Status::Ok or Status::DoorNotExist).
	///
	/// # Examples
	///
	/// ```
	/// let test = Door { id: 1, userid: None, connected: false};
	/// assert_eq!(test.connect("password"), Status::Ok);
	///
	/// ```
    fn connect(&mut self) -> Status {
		let connection = establish_connection();
        if doors.find(self.id).first(&connection) == Ok(self) {
            Status::Ok
        } else {
            Status::InvalidCreds
        }
	}
}

// Some custom HTTP headers
header! { (Topic, "topic") => [String] }
header! { (Password, "password") => [String] }

/// Provide MQTT authentification via HTTP
///
/// Every request coming to /user/{userid}/door/{doorid} will trigger this function.
fn query<'r, 'mw,'conn, D>(req: &'r mut Request<'mw, 'conn, D>) -> bool {
	if req.param("userid").is_none() || req.origin.headers.get_raw("password").is_none() {
		return false;
	}
	let headers = req.origin.headers;
	let id = req.param("doorid").unwrap().parse::<i64>().unwrap();
	let userid = req.param("userid").unwrap().parse::<i64>().unwrap();

    let _password = headers.get_raw("password").unwrap();
	let password = String::from_utf8(_password).unwrap();

	let topic: Option<String> = match headers.get_raw("topic") {
		Some(txt) => Some(String::from_utf8(txt).unwrap()),
		None => None
	};

	let mut door = Door{id: id, user_id: userid, password: password};

	match door.connect() {
		Status::DoorNotExist | Status::InvalidCreds => return false,
		Status::Ok => {}
	}
	// No topic, just plain authentication
	if topic.is_none() {
		return true;
	}

	if topic.unwrap() != String::from("user/") + &door.user_id.to_string() + "/door/" + &door.id.to_string() {
		return false;
	}
	true
}

// Because the MQTT broker only care about http status codes,
// we don't need to return json or whatever,
// so we just set the return HTTP code to 400 everywhere
fn handle_req<'r, 'mw,'conn, D>(req: &'r mut Request<'mw, 'conn, D>, res: Response<'mw, D>)
 -> MiddlewareResult<'mw, D> {
 	if query(req) {
		return Ok(Action::Continue(res));
	}
	return Err(NickelError::new(res, "", StatusCode::BadRequest));
}

fn main() {
    let mut server = Nickel::new();

    server.get("/user/:userid/door/:doorid", handle_req);

	server.listen("0.0.0.0:8080");
}

/*
// Horrible code but who cares ? Hell, it's tests after all !
#[cfg(test)]
mod test {
	use std::thread;
	use std::time::Duration;
	use super::sqlite;
	use std::fs;

	use hyper::Client;
	use hyper::header::Headers;
	use hyper::status::StatusCode;

	/// Prepare tests
	fn init_tests() {
		// We can run the webserver before populating the db because the server doesn't do any request on its own
		thread::spawn(move || {
            super::main();
        });
        let _ = fs::remove_file("sqlite.db");
        thread::sleep(Duration::from_millis(50));
        let connection = sqlite::open("sqlite.db").unwrap();
        connection.execute("
			CREATE TABLE doors (id INTEGER PRIMARY KEY ASC AUTOINCREMENT, user REFERENCES users (id) ON DELETE SET NULL, password TEXT NOT NULL);
			CREATE TABLE users (id INTEGER PRIMARY KEY ASC AUTOINCREMENT, username TEXT UNIQUE NOT NULL, mail TEXT UNIQUE NOT NULL, password TEXT NOT NULL);
			INSERT INTO users (id, username, mail, password) VALUES (1, 'yolo', 'yolo@incipient.fr', 'swag');
			INSERT INTO users (id, username, mail, password) VALUES (2, 'answer', 'answer@incipient.fr', '42');
			INSERT INTO doors (id, user, password) VALUES (1, 1, 'yolo1');
			INSERT INTO doors (id, user, password) VALUES (2, 1, 'yolo2');
			INSERT INTO doors (id, user, password) VALUES (3, 1, 'yolo3');
			INSERT INTO doors (id, user, password) VALUES (4, 2, '42-1');
			INSERT INTO doors (id, user, password) VALUES (5, 2, '42-2');
		").unwrap();
	}

	header! { (Topic, "topic") => [String] }
	header! { (Username, "username") => [String] }
	header! { (Password, "password") => [String] }

	macro_rules! headers {
		( $( $x:expr ),* ) => {
        {
			let mut headers = Headers::new();
            $(

				headers.set($x);
            )*
            headers
        }
    };
	}

	fn run_with_headers(id: i64, headers: Headers, assertion: StatusCode) {
		let client = Client::new();
		let res = client.get(&*("http://127.0.0.1:8080/doors/".to_owned() + &id.to_string())).headers(headers).send().unwrap();
		assert_eq!(res.status, assertion);
	}

	#[test]
	// Do not run each test independently because this would cause multiples servers to run at the same time
	// and the db migth be in a inconsistent state.
	fn test_everything() {
		init_tests();
		// missing password
		run_with_headers(1, headers!(Username("yolo".to_owned())), StatusCode::BadRequest);
		// missing username
		run_with_headers(1, headers!(Username("yolo".to_owned())), StatusCode::BadRequest);
		// Valid but no topic
		run_with_headers(1, headers!(Username("yolo".to_owned()), Password("swag".to_owned())), StatusCode::Continue);
		// Valid but no topic
		run_with_headers(1, headers!(Username("yolo".to_owned()), Password("swag".to_owned()), Topic("doors/1".to_owned())), StatusCode::BadRequest);
		// Invalid topic
		run_with_headers(1, headers!(Username("yolo".to_owned()), Password("swag".to_owned()), Topic("doors/2".to_owned())), StatusCode::BadRequest);
	}

}
*/
