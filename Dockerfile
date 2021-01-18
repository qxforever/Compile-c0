FROM gcc:8
WORKDIR /app/
COPY ./* ./
RUN g++ -Wall -Wextra -std=c++14 main.cpp -DONLINE_JUDGE -o program 
RUN chmod +x program
