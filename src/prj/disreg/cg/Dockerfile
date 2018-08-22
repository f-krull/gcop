FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
   npm \
   python2.7 \
   python-numpy \
   python-pandas \
   python-pip \
   python-scikits-learn \
   && rm -rf /var/lib/apt/lists/*

RUN pip install --upgrade clustergrammer

RUN npm install clustergrammer
