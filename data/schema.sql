CREATE TABLE IF NOT EXISTS objectTypes (
    id INTEGER PRIMARY KEY,
    mainType TEXT NOT NULL,
    subType TEXT NOT NULL,
    UNIQUE(mainType, subType)
);


CREATE TABLE IF NOT EXISTS objects (
    id INTEGER PRIMARY KEY,
    objectTypeId INTEGER NOT NULL,
);


CREATE TABLE IF NOT EXISTS objectStrings (
    id INTEGER PRIMARY KEY,
    objectId INTEGER NOT NULL,
    key TEXT NOT NULL,
    value TEXT NOT NULL,
    UNIQUE(objectId, key)
);


CREATE TABLE IF NOT EXISTS objectRelations (
    id INTEGER PRIMARY KEY,
    objectId INTEGER NOT NULL,
    relationType TEXT NOT NULL,
    relationId INTEGER NOT NULL,
    UNIQUE(objectId, relationType, relationId)
);


CREATE TABLE IF NOT EXISTS objectStats (
    id INTEGER PRIMARY KEY,
    objectId INTEGER NOT NULL,
    statType TEXT NOT NULL,
    value REAL NOT NULL,
    UNIQUE(objectId, statType)
);


CREATE TABLE IF NOT EXISTS objectEquip (
    id INTEGER PRIMARY KEY,
    objectId INTEGER NOT NULL,
    slot TEXT NOT NULL,
    itemId INTEGER NOT NULL,
    UNIQUE(objectId, slot)
);