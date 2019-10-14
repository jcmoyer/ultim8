#!/usr/bin/env sh

# Adapted from https://stackoverflow.com/a/151690

SCRIPTS_DIR=`dirname $(readlink -f "$0")`
SEARCH="http://www.apache.org/licenses/LICENSE-2.0"
FILECOUNT=0
ADDCOUNT=0

for i in $(find src include -name '*.hpp' -or -name '*.cpp')
do
  let "FILECOUNT++"
  if ! grep -q "$SEARCH" "$i"
  then
    printf "adding to $i\n"
    cat "$SCRIPTS_DIR/header.txt" "$i" > "$i.new" && mv "$i.new" "$i"
    let "ADDCOUNT++"
  fi
done

printf "scanned %d files\n" "$FILECOUNT"
printf "added headers to %d files\n" "$ADDCOUNT"
