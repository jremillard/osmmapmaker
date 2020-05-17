
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

CREATE VIRTUAL TABLE entitySpatialIndex USING rtree(
   id,              -- Integer primary key
   minX, maxX,      -- Minimum and maximum X coordinate
   minY, maxY       -- Minimum and maximum Y coordinate
);

CREATE INDEX entityKV_Index ON entityKV ( id, key, value); 
CREATE INDEX entity_Index ON entity (source, type); 




