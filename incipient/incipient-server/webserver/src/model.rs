use schema::users;
use schema::doors;

#[derive(Queryable)]
pub struct Door {
	id: i64,
	user_id: i64,
	password: String
}

#[insertable_into(doors)]
pub struct NewDoor<'a> {
    pub user_id: i64,
    pub password: &'a str
}

#[derive(Queryable)]
pub struct User {
    id: i64,
    username: String,
    mail: String,
    password: String
}

#[insertable_into(users)]
pub struct NewUser<'a> {
    pub username: &'a str,
    pub mail: &'a str,
    pub password: &'a str
}
