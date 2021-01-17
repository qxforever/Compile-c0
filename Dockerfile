FROM gcc:10
WORKDIR /Compile-c0/
COPY ./* ./
RUN g++ -Wall -Wextra -std=c++17 -O2 main.cpp -DONLINE_JUDGE -o program 
RUN chmod +x program