FROM ubuntu:18.04

RUN echo "Installing packages..." \
    && apt-get --quiet update --yes \
    && apt-get --quiet install git build-essential g++-8 cmake libboost-dev qtbase5-dev \
    libqt5core5a libqt5widgets5 libqt5printsupport5 qttools5-dev python python-pip graphviz doxygen --yes
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 40 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60
RUN pip install gcovr
RUN echo "Installing SOEM..." \
    && mkdir repos \
    && cd repos \
    && git clone https://gitlab.com/h2t/SOEM.git \
    && cd SOEM \
    && git checkout debian \
    && mkdir build \
    && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build . --target install 
RUN echo "Installing Catch2..." \
    && cd repos \
    && git clone https://github.com/catchorg/Catch2.git \
    && cd Catch2 \
    && git checkout v2.13.4 \
    && mkdir build \
    && cd build \
    && cmake -DBUILD_TESTING=OFF .. \
    && cmake --build . --target install
