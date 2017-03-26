import sqlite3
import cherrypy
from validate_email import validate_email
from utils import get_config, httpError, ok, parse_topic

OK = 0
INVALID_MAIL = 1
INSECURE_PASSWORD = 2
USER_NOT_EXIST = 3
USER_ALREADY_EXIST = 4
UNKNOWN_ERROR = 5
INVALID_CREDS = 6
DOOR_NOT_EXIST = 7
INVALID_USER = 8


class Server:
    def __init__(self):
        # TODO: TLS and change backend!!!
        conf = {
            '/': {
                'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
                'tools.response_headers.on': True,
                'tools.response_headers.headers': [('Content-Type', 'text/json')]
            }
        }
        cherrypy.config.update({'server.socket_host': get_config('auth_addr'),
                                'server.socket_port': int(get_config('auth_port')),
                                'environment': 'production',
                                'log.error_file': 'site.log'
                               })
        cherrypy.tree.mount(UsersService(), '/users', conf)
        # The API used by web server to ask for permission to a door according to a user
        cherrypy.tree.mount(User2DoorService(), '/users/doors', conf)
        # API used by the mosquitto auth broker
        cherrypy.tree.mount(DoorService(), '/doors', conf)

    def start(self):
        cherrypy.engine.start()

    def stop(self):
        cherrypy.engine.exit()


# Request from the web server
class User2DoorService:
    exposed = True

    @cherrypy.tools.json_in()
    def POST(self):
        try:
            json = cherrypy.request.json
            login = json['auth']['login']
            password = json['auth']['password']
            door_id = json['id']
        except:
            return httpError(200)

        user = User(login, password=password)
        exist = user.exist()
        if exist == USER_NOT_EXIST:
            return httpError(204)

        door = Door(door_id)
        exist = door.exist()
        if exist == DOOR_NOT_EXIST:
            return httpError(301)
        # User don't have permissions on this door
        if door.userid != user.userid:
            return httpError(207)
        return ok()


# Requests come from the mqtt broker
class DoorService:
    exposed = True

    # Because the broker only care about http status codes, we don't need to return json or whatever, so we just set the return HTTP code to 400
    def POST(self, username=None, password=None, topic=None, access=None, **kwargs):
        if not (username and password):
            # Credentials not supplied
            cherrypy.response.status = 400
            return

        door = Door(int(username))
        exist = door.exist()
        if exist == DOOR_NOT_EXIST:
            return httpError(301)
        connected = door.connect(password)
        if connected == INVALID_CREDS:
            cherrypy.response.status = 400
            return

        # topic not supplied but user logged in -> return an HTTP 200 message
        if not topic:
            return

        parsed = parse_topic(topic)
        # Invalid topic
        if not parsed:
            cherrypy.response.status = 400
            return
        if door.userid == parsed['userid'] and door.doorid == parsed['id']:
            return
        # User and door are not linked together -> invalid topic
        cherrypy.response.status = 400
        return


class Door:
    userid = -1
    doorid = -1
    connected = False

    def __init__(self, door_id: int):
        self.doorid = door_id

    def exist(self):
        with sqlite3.connect(get_config('sqlite_db')) as con:
            for door in con.execute("SELECT user FROM doors WHERE id=?", (self.doorid,)):
                self.userid = door[0]
                return OK
        return DOOR_NOT_EXIST

    # log in the door (for the mqtt broker)
    def connect(self, password: str):
        with sqlite3.connect(get_config('sqlite_db')) as con:
            for door in con.execute("SELECT user FROM doors WHERE id=? and password=?", (self.doorid, password)):
                self.connected = True
                return OK
        return INVALID_CREDS


class UsersService:
    exposed = True

    def GET(self, login, password):
        some = User(login, password=password)
        exist = some.exist()
        if exist == USER_NOT_EXIST:
            return httpError(204)
        return ok()

    def POST(self, username, mail, password):
        user = User(username, mail, password)
        exist = user.exist()
        # User already exist, so we couldn't add it another time
        if exist == OK:
            return httpError(205)

        valid = user.validate()
        if valid == INVALID_MAIL:
            return httpError(201)
        elif valid == INSECURE_PASSWORD:
            return httpError(202)

        state = user.insert_db()
        if state == UNKNOWN_ERROR:
            return httpError(203)
        return ok()


class User:
    name = ""
    password = ""
    mail = ""
    userid = -1
    valid = False

    def __init__(self, name, mail="", password=""):
        self.name = name
        self.mail = mail
        self.password = password

    def validate(self):
        if not validate_email(self.mail):
            return INVALID_MAIL
        if len(self.password) < 10:
            return INSECURE_PASSWORD
        self.valid = True
        return OK

    def present(self):
        return self.name, self.mail, self.password

    def exist(self):
        with sqlite3.connect(get_config('sqlite_db')) as con:
            for row in con.execute("SELECT id FROM users WHERE username=? AND password=?", (self.name, self.password)):
                self.userid = row[0]
                return OK
        return USER_NOT_EXIST

    def insert_db(self):
        if not self.valid:
            return INVALID_USER
        with sqlite3.connect(get_config('sqlite_db')) as con:
            try:
                con.execute("INSERT INTO users (username, mail, password) VALUES (?, ?, ?)", self.present())
                con.commit()
                return OK
            except sqlite3.IntegrityError:
                return USER_ALREADY_EXIST
            except:
                return UNKNOWN_ERROR
