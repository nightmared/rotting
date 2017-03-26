#!/usr/bin/env python3

import sqlite3
import sys
import pathlib
import os

if __name__ == "__main__":
	if len(sys.argv) > 1:
		p = pathlib.Path(sys.argv[1])
		if p.exists():
			os.remove(sys.argv[1])
		with sqlite3.connect(sys.argv[1]) as con:
			# Populate a clean database
			con.execute("CREATE TABLE doors (id INTEGER PRIMARY KEY ASC AUTOINCREMENT, user REFERENCES users (id) ON DELETE SET NULL, password TEXT NOT NULL);")
			con.execute("CREATE TABLE users (id INTEGER PRIMARY KEY ASC AUTOINCREMENT, username TEXT UNIQUE NOT NULL, mail TEXT UNIQUE NOT NULL, password TEXT NOT NULL);")
			con.commit()
	else:
		print("Please provide a filename !")
		exit(1)
