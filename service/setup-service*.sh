sudo cp ./matrix.service /etc/systemd/system

sudo systemctl daemon-reload

sudo systemctl enable matrix.service

sudo systemctl status matrix.service
