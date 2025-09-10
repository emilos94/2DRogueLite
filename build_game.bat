@echo off

mkdir .\build
pushd .\build

SET LINK_FLAGS=../dependencies/lib -lglfw3 -lopengl32 -lkernel32 -lgdi32 -luser32 -lOpenAL32

gcc -g -I ../dependencies/include -I ../src glad.o all_engine.o ../src/game/all_game.c ../src/main.c -o main.exe -L %LINK_FLAGS%

popd
