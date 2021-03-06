/* Media table definitions for SQLite database
 *
 * Project : minidlna
 * Website : http://sourceforge.net/projects/minidlna/
 * Author  : Douglas Carmichael
 *
 * MiniDLNA media server
 * Copyright (C) 2008-2009  Justin Maggard
 *
 * This file is part of MiniDLNA.
 *
 * MiniDLNA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MiniDLNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MiniDLNA. If not, see <http://www.gnu.org/licenses/>.
 */

const char * db_schema =
		"CREATE TABLE OBJECTS ("
		"	ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"	OBJECT_ID TEXT UNIQUE NOT NULL, "
		"	PARENT_ID TEXT NOT NULL, "
		"	REF_ID TEXT DEFAULT NULL, "
		"	CLASS TEXT NOT NULL, "
		"	DETAIL_ID INTEGER DEFAULT NULL, "
		"	NAME TEXT DEFAULT NULL"
		");"

		"CREATE TABLE DETAILS ("
		"	ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"	PATH TEXT DEFAULT NULL, "
		"	SIZE INTEGER, "
		"	TIMESTAMP INTEGER, "
		"	TITLE TEXT COLLATE NOCASE, "
		"	DURATION TEXT, "
		"	BITRATE INTEGER, "
		"	SAMPLERATE INTEGER, "
		"	CREATOR TEXT COLLATE NOCASE, "
		"	ARTIST TEXT COLLATE NOCASE, "
		"	AUTHOR TEXT COLLATE NOCASE, "
		"	ALBUM TEXT COLLATE NOCASE, "
		"	GENRE TEXT COLLATE NOCASE, "
		"	RATING TEXT, "
		"	COMMENT TEXT, "
		"	DESCRIPTION TEXT, "
		"	CHANNELS INTEGER, "
		"	DISC INTEGER, "
		"	TRACK INTEGER, "
		"	DATE DATE, "
		"	RESOLUTION TEXT, "
		"	THUMBNAIL BOOL DEFAULT 0, "
		"	ALBUM_ART INTEGER DEFAULT 0, "
		"	MTA INTEGER DEFAULT 0, "
		"	VIDEO_TYPE INTEGER DEFAULT 0, "
		"	ROTATION INTEGER, "
		"	DLNA_PN TEXT, "
		"	MIME TEXT"
		");"

		"CREATE TABLE ALBUM_ART ("
		"	ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"	PATH TEXT NOT NULL);"

		"CREATE TABLE CAPTIONS ("
		"	ID INTEGER PRIMARY KEY, "
		"	PATH TEXT NOT NULL"
		");"

		"CREATE TABLE BOOKMARKS ("
		"	ID INTEGER PRIMARY KEY, "
		"	SEC INTEGER"
		");"

		"CREATE TABLE MTA ("
		"	ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"	PATH TEXT NOT NULL"
		");"

		"CREATE TABLE SETTINGS ("
		"	KEY TEXT NOT NULL, "
		"	VALUE TEXT"
		");";


