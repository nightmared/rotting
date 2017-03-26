#[macro_use] extern crate nickel;
extern crate rustc_serialize;
extern crate csv;

use rustc_serialize::json;

use nickel::{Nickel, HttpRouter};
use nickel::status::StatusCode;

mod door;
mod user;

fn main() {
    let mut server = Nickel::new();

	// Because the MQTT broker only care about http status codes,
	// we don't need to return json or whatever,
	// so we just set the return HTTP code to 400 everywhere
    server.post("/doors/login", middleware! { |req, mut res|
		if !door::auth(req) {
			res.set(StatusCode::BadRequest);
		}
		""
	});
	server.post("/doors/check", middleware! { |req, mut res|
		if !door::verify_credentials(req) {
			res.set(StatusCode::BadRequest);
		}
		""
	});
	server.post("/doors/get_passwd", middleware! { |req, mut res|
		match door::get_passwd(req) {
			Some(expr) => format!("{{\"status\": 0, \"value\": \"{}\"}}", expr),
			None => { res.set(StatusCode::BadRequest); "{\"status\": 1}".to_owned() }
		}
	});

	server.post("/users/login", middleware! { |req, mut res|
		match user::auth(req) {
			Some(user) => json::encode(&user).unwrap(),
			None => { res.set(StatusCode::BadRequest); "".to_owned() }
		}
	});
	server.listen("0.0.0.0:8080");
}
