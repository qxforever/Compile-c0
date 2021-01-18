FROM gcc:10
WORKDIR /app/
COPY ./* ./
RUN g++ -Wall -Wextra -std=c++17 main.cpp -DONLINE_JUDGE -o program 
RUN chmod +x program
