
create table point ( id integer primary key autoincrement, source string, geom blob);
create table line ( id integer primary key autoincrement, source string, geom blob);
create table area ( id integer primary key autoincrement, source string, geom blob);

create table pointKV ( 
	id integer, 
	key string, 
	value string,

	CONSTRAINT fk_column
		FOREIGN KEY (id)
		REFERENCES point (id)
		ON DELETE CASCADE

	);

create table lineKV ( 
	id integer, 
	key string, 
	value string,

	CONSTRAINT fk_column
		FOREIGN KEY (id)
		REFERENCES line (id)
		ON DELETE CASCADE

	);

create table areaKV ( 
	id integer, 
	key string, 
	value string,

	CONSTRAINT fk_column
		FOREIGN KEY (id)
		REFERENCES area (id)
		ON DELETE CASCADE
	);

CREATE UNIQUE INDEX pointKV_Index ON pointKV (key, value ); 
CREATE UNIQUE INDEX lineKV_Index ON lineKV (key, value ); 
CREATE UNIQUE INDEX areaKV_Index ON areaKV (key, value ); 




