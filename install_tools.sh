# Run install_tools.sh with sudo:
#
# $ sudo ./install_tools.sh
#
dnf update
dnf install -y g++ make openssl libssl-devel wget boost-devel
