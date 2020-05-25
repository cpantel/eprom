for file in $( ls -1 chip*.dump ); do
  ./bitCount < "$file"
  md5sum "$file"
done
