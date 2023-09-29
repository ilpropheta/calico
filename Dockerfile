FROM gcc

LABEL description="calico builder" 
RUN apt-get update && apt-get install -y rsync zip openssh-server make ninja-build cmake gdb bison build-essential libopencv-dev

# we stick with this version of vcpkg just to be sure that it works. Feel free to upgrade
ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone https://github.com/Microsoft/vcpkg.git --depth 1 --branch 2023.08.09 $VCPKG_ROOT \
 && $VCPKG_ROOT/bootstrap-vcpkg.sh \
 && ln -s $VCPKG_ROOT/vcpkg /usr/local/bin

# this is in case you want to reach the container via SSH (e.g. from Visual Studio)
RUN mkdir -p /var/run/sshd
RUN echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config && \ 
   ssh-keygen -A 
EXPOSE 22
RUN useradd -m -d /home/marco -s /bin/bash -G sudo marco
RUN echo "marco:marco" | chpasswd

# required dependencies
RUN vcpkg install sobjectizer
RUN vcpkg install grpc

# launch the ssh service just before starting
ENTRYPOINT service ssh start && bash