sudo cp ./matrix.service /etc/systemd/system
sudo cp ./server-matrix.service /etc/systemd/system

sudo systemctl daemon-reload

sudo systemctl enable matrix.service
sudo systemctl enable server-matrix.service

sudo systemctl status matrix.service
sudo systemctl status server-matrix.service