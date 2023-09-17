# pg_badapple

Play "Bad Apple!!" in the psql client. This demo is to illustrate that the psql client can play some animation.

## Install

```bash
make PG_CONFIG=<path to pg_config> install
```

## Run

```sql
-- In psql client.
CREATE EXTENSION badapple;
SELECT play_badapple();
```

## Reference

- [Fun with PostgreSQL Puzzles: Recursive Functions with Animations](https://www.crunchydata.com/blog/fun-with-postgresql-puzzles-and-recursive-functions-with-animations)
- [ANSI/VT100 Terminal Control Escape Sequences](https://web.archive.org/web/20190624214929/http://www.termsys.demon.co.uk/vtansi.htm)
- [Chion82/ASCII_bad_apple](https://github.com/Chion82/ASCII_bad_apple)
