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

    static clearDraw(){
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
            for(var i = 0; i < pixelToSend.length; i++) {
                if(pixelToSend[i] != null) {
                    pixelToSendOrdered.push(pixelToSend[i]);
                }
            }
            //console.log(pixelToSendOrdered);

            dataJSON.COLOR = hexToRGB(drawingColor);
            dataJSON.LEDS = pixelToSendOrdered;
            
            pixelToSend = [];
            if(pixelToSendOrdered.length == 0){

            }else{
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

    // Creates a promise object for sending the desired data
    fetch(window.location.href + 'post', {
        method: "POST",
        // Format of the body must match the Content-Type
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(data)
    });
}

function drawMsg(data) {

    // Creates a promise object for sending the desired data
    fetch(window.location.href + 'draw', {
        method: "POST",
        // Format of the body must match the Content-Type
        headers: { "Content-Type": "text/plain" },
        body: data
    });
}



// function sendData(data) {
//     var XHR = new XMLHttpRequest();
//     var FD = new FormData();
//     // Mettez les données dans l'objet FormData
//     for (name in data) {
//         FD.append(name, data[name]);
//     }

//     // Définissez ce qui se passe si la soumission s'est opérée avec succès
//     XHR.addEventListener('load', function(event) {});

//     // Definissez ce qui se passe en cas d'erreur
//     XHR.addEventListener('error', function(event) {});

//     // Configurez la requête
//     XHR.open('POST', window.location.href + 'post');

//     // Expédiez l'objet FormData ; les en-têtes HTTP sont automatiquement définies
//     XHR.send(FD);
// }