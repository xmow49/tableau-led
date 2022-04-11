// function sendMode (){
//     var valeurButton = document.querySelector('input[name="animation-selector"]:checked').value;
//     var mode;
//     var animationSpeed = document.getElementById("animation-speed").value;
//     if (paint == true)
//         mode = "DRAW";
//     else
//         mode = "GIF";
//     console.log(valeurButton);
//     console.log(mode);
//     console.log(animationSpeed);
//     var data = mode + "," + valeurButton + "," + animationSpeed;
//  
// }

var data;
var animationSpeed;
var valeurButton;
var mode;
var pixelToSend = [];

var tempMatrix = [
    [],
    []
];


var dataJSON = {
    TO: 'CPP',
    MODE: "GIF",
};

class send {
    static mode() {
        if (drawing == true)
            mode = "DRAW";
        else
            mode = "GIF";
        //console.log(mode);
        return mode;
    }

    static buttonValue() {
        valeurButton = document.querySelector('input[name="animation-selector"]:checked').value;
        // console.log(valeurButton);
        return valeurButton;
    }

    static animationSpeed() {
        animationSpeed = document.getElementById("animation-speed").value;
        // console.log(animationSpeed);
        return animationSpeed;
    }

    static clearDraw() {
        dataJSON = {
            TO: 'CPP',
            MODE: "DRAW",
            DRAW: 'CLEAR'
        };
        postMsg(dataJSON);

    }

    static dataBuild() {
        dataJSON = {
            TO: 'CPP',
            MODE: send.mode()
        };
        if (send.mode() == "DRAW") {
            //console.log(pixelToSend);
            var pixelToSendOrdered = [];
            for (var i = 0; i < pixelToSend.length; i++) {
                if (pixelToSend[i] != null) {
                    pixelToSendOrdered.push(pixelToSend[i]);
                }
            }
            //console.log(pixelToSendOrdered);

            dataJSON.COLOR = hexToRGB(drawingColor);
            dataJSON.LEDS = pixelToSendOrdered;

            pixelToSend = [];
            if (pixelToSendOrdered.length == 0) {

            } else {
                console.log(dataJSON);
                postMsg(dataJSON);
            }



        } else if (send.mode() == "GIF") {
            dataJSON.GIF = send.buttonValue();
            dataJSON.SPEED = send.animationSpeed();
            console.log(dataJSON);
            postMsg(dataJSON);

        }


    }

}


function dec2bin(dec) {
    return (dec >>> 0).toString(2);
}
const equals = (a, b) =>
    a.length === b.length &&
    a.every((v, i) => v === b[i]);

// Sends message to server via POST
function postMsg(data) {

    // // Creates a promise object for sending the desired data
    // fetch(window.location.href + 'post', {
    //     method: "POST",
    //     // Format of the body must match the Content-Type
    //     headers: { "Content-Type": "application/json" },
    //     body: JSON.stringify(data);
    // });

    socket.send(JSON.stringify(data));
}



// Créer une connexion WebSocket
const domain = (new URL(window.location.href));
var socket;

class socketConnection {

    static connect() {
        document.getElementById("connect").style.display = "flex";
        document.getElementById("disconected").style.display = "none";
        document.getElementById("popup-background").style.display = "block";

        socket = new WebSocket('ws://' + domain.hostname + ":8083");
        // La connexion est ouverte
        socket.addEventListener('open', function (event) {
            console.log("Connexion établie");
            document.getElementById("connect").style.display = "none";
            document.getElementById("disconected").style.display = "none";
            document.getElementById("popup-background").style.display = "none";
        });

        // Écouter les messages
        socket.addEventListener('message', function (event) {
            console.log('MSG FROM SERVER:', event.data);
        });

        socket.addEventListener('close', function (event) {
            document.getElementById("connect").style.display = "none";
            document.getElementById("disconected").style.display = "flex";
            document.getElementById("popup-background").style.display = "block";
        });

        socket.addEventListener('error', function (event) {
            console.log('ERROR:', event.data);
        });
    }
}

socketConnection.connect();

