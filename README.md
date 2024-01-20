# prodDBbackup
Utility for mass backup of schemes from OracleDB. Works in multithreaded mode: part of the streams are engaged in exporting data from the database, part is engaged in obtaining data saving in compressed form to disk
The zlib library is used for compression - https://github.com/madler/zlib

Usage: prodDBbackup login/pass@dblink dumpdir=C:\dir\to\backup [schemaset="Select ...." jsonreportfile=C:\path\to\file consistent=no|yes]
schemaset is a request from the database that returns a set of schemes for backup (ex. select username from dba_users;)
consistent is equivalent \"flashback_time=systimestamp\" option for expdp, "no" is default
For multithread export create PRODDBBACKUP_DIR_1, PRODDBBACKUP_DIR_2... directories on your DB server