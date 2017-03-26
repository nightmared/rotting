import unittest
import sqlite3
import os
import pathlib
import requests

class UserTest:
    def __init__(self, id, login, mail, password):
        self.id = id
        self.username = login
        self.mail = mail
        self.password = password

    def present(self):
        return self.id, self.username, self.mail, self.password


class DoorTest:
    def __init__(self, id, userid, password):
        self.id = id
        self.userid = userid
        self.password = password

    def present(self):
        return self.id, self.userid, self.password

    def gentopic(self):
        return 'users/' + str(self.userid) + '/doors/' + str(self.id)

user1 = UserTest(1, 't0b1nux', 'test@gmaile.com', 'thobyrhgfyujtiyutry')
user2 = UserTest(2, 't0b1n5ux', 'test@gmaile2.com', 'thobyrvbghy')
door1 = DoorTest(1, 1, 'tryhxdsxcgdvhgfj')
door2 = DoorTest(2, 1, 'trdgfdy')
door3 = DoorTest(3, 2, 'trdfsgxtdfgy')


class TestAuthServer(unittest.TestCase):
    base_url = 'http://127.0.0.1:8080/'

    def testdoor_auth(self):
        url = self.base_url + 'doors/'

        # Do not provide the 'topic' and 'acc' information but still valid
        r = requests.post(url, data={"username": door1.id, "password": door1.password})
        self.assertEqual(r.status_code, 200)
        r = requests.post(url, data={"username": door3.id, "password": door3.password})
        self.assertEqual(r.status_code, 200)

        r = requests.post(url, data={"username": door1.id, "password": door1.password, "topic": door1.gentopic()})
        self.assertEqual(r.status_code, 200)
        r = requests.post(url, data={"username": door3.id, "password": door3.password, "topic": door3.gentopic()})
        self.assertEqual(r.status_code, 200)
        r = requests.post(url, data={"username": door1.id, "password": door1.password, "acc": 1})
        self.assertEqual(r.status_code, 200)

        # Malformed topic
        r = requests.post(url, data={"username": door3.id, "password": door3.password, "topic": "usfyghjlkm*ers/48//frg/doors/" + str(door3.id)})
        self.assertEqual(r.status_code, 400)

        # Invalid user in the topic
        r = requests.post(url, data={"username": door3.id, "password": door3.password, "topic": "users/" + str(door1.userid) + "/doors/" + str(door3.id)})
        self.assertEqual(r.status_code, 400)

        # Invalid door in the topic
        r = requests.post(url, data={"username": door3.id, "password": door3.password, "topic": "users/" + str(door3.userid) + "/doors/" + str(door1.id)})
        self.assertEqual(r.status_code, 400)

        # Invalid password
        r = requests.post(url, data={"username": door3.id, "password": "random-shitty-password", "topic": door3.gentopic()})
        self.assertEqual(r.status_code, 400)

        # Invalid username
        r = requests.post(url, data={"username": door1.id, "password": door3.password, "topic": door3.gentopic()})
        self.assertEqual(r.status_code, 400)

        # Do not use the good auth method (BASIC HTTP instead of POST credentials)
        r = requests.get(url, auth=('995', 'try'))
        self.assertEqual(r.status_code, 405)

    def test_user_auth(self):
        url = self.base_url + 'users/'

        r = requests.get(url, params={"login": user1.username, "password": user1.password})
        self.assertEqual(r.status_code, 200)

        # lack of parameters in the query
        r = requests.get(url, params={"login": user1.username})
        self.assertEqual(r.status_code, 404)
        r = requests.get(url, params={"password": user1.password})
        self.assertEqual(r.status_code, 404)

        # Invalid HTTP method
        r = requests.post(url, params={"login": user1.username, "password": user1.password})
        self.assertEqual(r.status_code, 404)

    def test_user_add(self):
        url = self.base_url + 'users/'

         # Missing argument
        r = requests.post(url, params={"login": 't0b1nuxe', "password": "thobyrhyujtiyutry"})
        self.assertEqual(r.status_code, 404)

        # Wrong parameter: 'login' instead of 'username'
        r = requests.post(url, params={"login": 't0b1nuxe', "password": "thobyrhyujtiyutry", "mail": "fgjfg@test.fr"})
        self.assertEqual(r.status_code, 404)

        # Wrong mail
        r = requests.post(url, params={"username": 't0b1nuxe', "password": "thobyrhyujtiyutry", "mail": "fgjfg,fld"})
        self.assertEqual(r.status_code, 404)

        # User already exist
        r = requests.post(url, params={"username": user1.username, "password": user1.password, "mail": "fgjfg@test.fr"})
        self.assertEqual(r.status_code, 400)

        r = requests.post(url, params={"username": 't0b1nuxe', "password": "thobyrhyujtiyutry", "mail": "fgjfg@test.fr"})
        self.assertEqual(r.status_code, 200)

    def test_users_door_auth(self):
        url = self.base_url + 'users/doors/'
        r = requests.post(url, json={'auth': {'login': user1.username, 'password': user1.password}, 'id': door1.id})
        self.assertEqual(r.status_code, 200)

        # Invalid login
        r = requests.post(url, json={'auth': {'login': 't0b1nuxcxgtuy', 'password': user1.password}, 'id': door1.id})
        self.assertEqual(r.status_code, 404)
        # Invalid password
        r = requests.post(url, json={'auth': {'login': user1.username, 'password': 'thobyrhyujttyrgjretggfhughuiyutry'}, 'id': door1.id})
        self.assertEqual(r.status_code, 404)
        # Invalid id
        r = requests.post(url, json={'auth': {'login': user1.username, 'password': user1.password}, 'id': 12748})
        self.assertEqual(r.status_code, 404)
        # Id doesn't match the user
        r = requests.post(url, json={'auth': {'login': user1.username, 'password': user1.password}, 'id': door3.id})
        self.assertEqual(r.status_code, 401)

# TODO: client tests
if __name__ == '__main__':
    p = pathlib.Path("db.db")
    if p.exists():
        os.remove('db.db')
    with sqlite3.connect('db.db') as con:
        def adduser(user):
            con.execute("INSERT INTO users (id, username, mail, password) VALUES (?, ?, ?, ?);", user.present())
        def adddoor(door):
            con.execute("INSERT INTO doors (id, user, password) VALUES (?, ?, ?);", door.present())

        # Populate database with predefined data
        con.execute("CREATE TABLE doors (id INTEGER PRIMARY KEY ASC AUTOINCREMENT, user REFERENCES users (id) ON DELETE SET NULL, password TEXT NOT NULL);")
        con.execute("CREATE TABLE users (id INTEGER PRIMARY KEY ASC AUTOINCREMENT, username TEXT UNIQUE NOT NULL, mail TEXT UNIQUE NOT NULL, password TEXT NOT NULL);")
        adduser(user1)
        adduser(user2)

        adddoor(door1)
        adddoor(door2)
        adddoor(door3)
        con.commit()

    unittest.main()
    p = pathlib.Path('db.db')
    if p.exists():
        os.remove('db.db')
