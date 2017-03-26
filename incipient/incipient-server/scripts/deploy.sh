#!/bin/sh -e

# /!\ This program sould be launch from the root of the git repo, not from the scripts directory !
base=$(pwd)
out=$base/run-native-env
VENV="/usr/bin/virtualenv-3.5"

setup_venv() {
	echo "--> Deploying a new Python3 virtual environment"

	venvDir="$out/venv"
	[ -d "$venvDir" ] && rm -rf $venvDir

	echo "*** Creating new virtualenv in folder $venvDir"
	$VENV $venvDir > /dev/null

	echo "*** Activating virtualenv"
	. $venvDir/bin/activate
	for package in "requests" "paho-mqtt" "cherrypy" "validate_email"
	do
		pip3 install $package > /dev/null
		echo "*** $package installed"
	done
}

deps() {
	echo "--> Compiling dependencies (SQlite3, mosquitto and mosquitto-auth-plugin-http)"
	mkdir $out/tmp
	cd $out/tmp

	sqlite() {
		name=SQLite3
		# We compile sqlite ourselves because we need to add foreign key support
		echo "*** Downloading $name"
		file=sqlite-amalgamation-201602090212
		wget https://sqlite.org/snapshot/$file.zip -q
		unzip $file > /dev/null

		echo "*** Compiling $name"
		libname=libsqlite3.so.0.8.6
		gcc -O2 -DSQLITE_THREADSAFE=0 -DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DSQLITE_ENABLE_EXPLAIN_COMMENTS -DHAVE_USLEEP -DHAVE_READLINE -DSQLITE_DEFAULT_FOREIGN_KEYS=1 -ldsl -lreadline -lncurses -c sqlite3.c -fPIC -DPIC -o sqlite3.o
		gcc -shared -fPIC -DPIC sqlite3.o -ldl -lpthread -O2 -Wl,-soname -Wl,libsqlite3.so.0 -o $libname

		# Install sqlite3
		echo "*** Installing $name"
		cd $out/libs
		cp $out/tmp/$libname .
		ln -s $libname libsqlite3.so.0
		ln -s $libname libsqlite3.so
	}

	mosquitto() {
		name=mosquitto
		echo "*** Downloading $name"
		version=1.4.7
		wget -q http://mosquitto.org/files/source/mosquitto-$version.tar.gz
		tar xf mosquitto-$version.tar.gz

		echo "*** Compiling $name"
		cd mosquitto-$version
		make -j6 > /dev/null 2>&1

		echo "*** Installing $name"
		cp lib/libmosquitto.so.1 $out/libs/
		cp src/mosquitto $out/bin/

		echo "*** Configurig $name"
		cat > $out/mosquitto.conf << EOF
log_type debug
listener 1883
auth_plugin $out/libs/mosquitto_auth_plugin_http.so
allow_anonymous false
allow_duplicate_messages true
EOF
		
		cd $out/tmp
		
		name=mosquitto_auth_plugin_http
		echo "*** Downloading $name"
		wget -q https://raw.githubusercontent.com/hadleyrich/mosquitto-auth-plugin-http/master/$name.c
		sed 's_(#define DEFAULT_USER_URI "http://localhost):5000/mqtt-user"_\1:8080/doors"_' $name.c
		sed 's_(#define DEFAULT_ACL_URI "http://localhost):5000/mqtt-user"_\1:8080/doors"_' $name.c

		echo "*** Compiling $name"
		cflags="-Wall -Werror -fPIC -I. -Imosquitto-$version/lib -Imosquitto-$version/src"
		gcc  -c $cflags $name.c -o $name.o
		gcc $cflags -shared $name.o -o $name.so -lcurl

		echo "*** Installing $name"
		cp $name.so $out/libs/
	}

	sqlite
	cd $out/tmp
	mosquitto
	# Cleanup
	rm -rf $out/tmp
}

populate_db() {
	echo "--> Populate new database"
	LD_LIBRARY_PATH="$out/libs:$LD_LIBRARY_PATH" $base/scripts/init-db.py "$out/db.db"
}

rm -rf $out && mkdir $out && cd $out

echo "--> Creating new folders in $out"
mkdir libs bin

echo "--> Copying necessary data and programs"
find $base/pyfiles/ -name '*.py' -exec cp -t $out \{\} \;
cp $base/scripts/run-server.sh run.sh
chmod +x run.sh
cp $base/Error.csv .
# Update database file with new db location
sed -nr '/sqlite_db/ !p' < $base/Config.csv > Config.csv
echo "sqlite_db,$out/db.db" >> Config.csv

setup_venv
deps
populate_db
echo "Congratulation, incipientServer had been configured and installed!"
