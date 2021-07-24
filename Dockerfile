FROM ubuntu:18.04
WORKDIR /app
COPY . .
RUN apt-get update && apt-get install -y gcc g++ cmake make
RUN cmake -Bbuild -DCMAKE_BUILD_TYPE=Release . && cd /app && make
CMD ./server --http=3000 --www=/app/static_site
