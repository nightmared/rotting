use csv;
use nickel::{JsonBody, Request};

#[derive(RustcDecodable, RustcEncodable, PartialEq)]
pub struct User {
    pub id: i64,
    username: String,
    mail: String,
    password: String
}

impl User {
    pub fn exists(name: &str, pass: &str) -> Option<User> {
        let mut rdr = csv::Reader::from_file("users.db")
			.unwrap()
			.has_headers(false);
        for db_row in rdr.decode().collect::<csv::Result<Vec<User>>>().unwrap() {
            if db_row.username == name && &db_row.password == pass {
                return Some(db_row);
            }
        }
        None
    }
}

pub fn auth<'r, 'mw,'conn, D>(req: &'r mut Request<'mw, 'conn, D>) -> Option<User> {
    #[derive(RustcDecodable)]
    struct Query {
        username: String,
        password: String
    }

    let query = match req.json_as::<Query>() {
        Ok(some) => some,
        Err(_) => return None
    };

    User::exists(&query.username, &query.password)
}
