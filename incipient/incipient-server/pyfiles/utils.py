import csv
import cherrypy
from enum import Enum
import os
from datetime import datetime


# topic is a string that we assume to be in the following form
def parse_topic(topic: str):
    out = {}
    try:
        out['userid'] = int(topic.split('/')[1])
        out['id'] = int(topic.split('/')[3])
        return out
    # Out of bounds or something like that
    except:
        return None


def parse_commands(string: str):
    out = {}
    for i in string.decode("utf-8").split(','):
        split = i.split(':')
        out[split[0]] = split[1]
    return out


def get_config(property: str):
    with open(os.path.dirname(os.path.realpath(__file__)) + '/Config.csv') as csvfile:
        reader = csv.DictReader(csvfile, delimiter=',')
        for row in reader:
            if row['key'] == property:
                return row['value']


# Errors are handled this way :
# an exception is thrown, then catch in a try-except block
# The except block run httpError with an error code corresponding to a message stored in Error.csv
def httpError(errcode):
    with open(os.path.dirname(os.path.realpath(__file__)) + '/Error.csv') as csvfile:
        reader = csv.DictReader(csvfile, delimiter=',')
        for row in reader:
            if row['errcode'] == str(errcode):
                cherrypy.response.status = row['httpstatus']
                return "{'status':'error','errcode':" + row['errcode'] + ",'msg':'" + row['description'] + "'}"
        return httpError(500)


def ok():
    return "{'status':'ok'}"


class LogLevel(Enum):
    debug = "DEBUG"
    notice = "NOTICE"
    warning = "WARNING"
    error = "ERROR"
    fatal = "FATAL"

    def str(self):
        return self._value_


def log(loglevel, msg):
    file = open('log', 'a')
    out = datetime.now().__str__() + ": " + loglevel.str() + ": " + msg
    file.write(out + "\n")
    print(out)
