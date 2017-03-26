use csv;
use nickel::{JsonBody, Request};
use user::User;

#[derive(RustcDecodable, RustcEncodable, PartialEq)]
struct Door {
	id: i64,
	user_id: i64,
	password: String
}

macro_rules! getRdr {
	() => {
		csv::Reader::from_file("doors.db")
			.unwrap()
			.has_headers(false)
	};
}

impl Door {
	fn get_user_id(id: i64) -> i64 {
		let mut rdr = getRdr!();
		for db_row in rdr.decode().collect::<csv::Result<Vec<Door>>>().unwrap() {
			if db_row.id == id {
				return db_row.user_id;
			}
		}
		-1
	}

    fn get_password(id: i64) -> Option<String> {
		let mut rdr = getRdr!();
		for db_row in rdr.decode().collect::<csv::Result<Vec<Door>>>().unwrap() {
			if db_row.id == id {
				return Some(db_row.password);
			}
		}
		None
	}

	fn exists(id: i64, pass: &str) -> bool {
		let mut rdr = getRdr!();
		for db_row in rdr.decode().collect::<csv::Result<Vec<Door>>>().unwrap() {
			if db_row.id == id && &db_row.password == pass {
				return true;
			}
		}
		false
	}
}


// just check username/password couple is valid
pub fn verify_credentials<'r, 'mw,'conn, D>(req: &'r mut Request<'mw, 'conn, D>) -> bool {
	#[derive(RustcDecodable)]
	struct Query {
		id: i64,
		password: String
	}

	let query = match req.json_as::<Query>() {
		Ok(some) => some,
		Err(_) => return false
	};

	Door::exists(query.id, &query.password)
}

pub fn auth<'r, 'mw,'conn, D>(req: &'r mut Request<'mw, 'conn, D>) -> bool {
	#[derive(RustcDecodable)]
	struct Query {
		id: i64,
		topic: String
	}

	let query = match req.json_as::<Query>() {
		Ok(some) => some,
		Err(_) => return false
	};

	let userid = Door::get_user_id(query.id);
	if userid == -1 {
		return false;
	}
	let expected = format!("{}{}{}{}", String::from("user/"), userid, "/door/", query.id);
	query.topic == expected
}

pub fn get_passwd<'r, 'mw,'conn, D>(req: &'r mut Request<'mw, 'conn, D>) -> Option<String> {
    #[derive(RustcDecodable)]
    pub struct Auth {
        pub username: String,
        pub password: String
    }

    #[derive(RustcDecodable)]
	struct Query {
        auth: Auth,
		id: i64
	}

    let query = match req.json_as::<Query>() {
        Ok(some) => some,
        Err(_) => return None
    };

    let user = User::exists(&query.auth.username, &query.auth.password);

    if user.is_some() && user.unwrap().id == Door::get_user_id(query.id) {
        Door::get_password(query.id)
    } else {
        None
    }
}
