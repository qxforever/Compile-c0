FROM gcc:10
WORKDIR /Compile-c0/
COPY ./* ./
RUN g++ -Wall -Wextra -std=c++14 -O2 -fsanitize=address main.cpp -o program
RUN chmod +x program
