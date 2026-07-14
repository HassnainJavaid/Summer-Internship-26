@echo off
echo Building GPT Tokenizer...

:: Compile object files
gcc -Wall -Wextra -O2 -g -Iinclude -c src/hashmap.c -o obj/hashmap.o
gcc -Wall -Wextra -O2 -g -Iinclude -c src/merge_table.c -o obj/merge_table.o
gcc -Wall -Wextra -O2 -g -Iinclude -c src/tokenizer_loader.c -o obj/tokenizer_loader.o

:: Compile tests
gcc -Wall -Wextra -O2 -g -Iinclude obj/hashmap.o obj/merge_table.o obj/tokenizer_loader.o tests/test.c -o bin/test_loader -lm

echo Build complete!
echo Binaries are in bin/ directory
