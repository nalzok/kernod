PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS users(
  user_id   INTEGER PRIMARY KEY,
  username  TEXT NOT NULL UNIQUE,
  email     TEXT NOT NULL UNIQUE,
  password  TEXT NOT NULL,
  join_ts   TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
)