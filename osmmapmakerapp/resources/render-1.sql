
create table entity ( 
	id integer primary key autoincrement, 
	source string, 
	type integer, 
	geom blob
	);

create table entityKV ( 
	id integer, 
	key string, 
	value string,

	CONSTRAINT fk_column
		FOREIGN KEY (id)
		REFERENCES entity (id)
		ON DELETE CASCADE
	);

CREATE INDEX entityKV_Index ON entityKV ( key, value ); 
CREATE INDEX entity_Index ON entity (source, type); 




