PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS users(
  user_id       INTEGER PRIMARY KEY,
  username      TEXT NOT NULL UNIQUE,
  email         TEXT NOT NULL UNIQUE,
  password      TEXT NOT NULL,
  join_ts       TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS questions(
  question_id   INTEGER PRIMARY KEY,
  title         TEXT NOT NULL,
  body          TEXT NOT NULL,
  author_id     INTEGER REFERENCES users,
  post_ts       TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
