var drawingState = false;
var matrixSize = 128;

var colorArray = new Array(3).fill(0);

var matrixPixels = new Array(matrixSize * matrixSize).fill(colorArray);

function hexToRGB(hex) {
    hex = hex.replace('#', '');
    var r = parseInt(hex.substring(0, 2), 16);
    var g = parseInt(hex.substring(2, 4), 16);
    var b = parseInt(hex.substring(4, 6), 16);
    var rgb = [r, g, b];
    return rgb;
}

function printMatrix() {
    var matrixString = "";
    for (var i = 0; i < matrixSize; i++) {
        for (var j = 0; j < matrixSize; j++) {
            matrixString += matrixPixels[i * matrixSize + j]; + "";
        }
        matrixString += "\n";
    }
    console.log(matrixString);
}

function map(x, in_min, in_max, out_min, out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

function setPixelMatrix(x, y) {
    if(x > 128 || y < 0 || y > 128 || y < 0){
        return;
    }
    var cellID = (matrixSize * y + x);
    if (cellID < matrixSize * matrixSize && cellID >= 0) {
        // document.getElementById('cell-' + cellID).style.backgroundColor = drawingColor;
        matrixPixels[cellID] = hexToRGB(drawingColor);
        pixelToSend[cellID] = cellID; //éviter les doublons
        // console.log("matrixPixels[" + cellID + "] = " + matrixPixels[cellID]);
    }

}

function drawCircle(cx, cy, r) {
    var dim = 100

    for (let i = -r; i <= r; i += 1) {
        for (let j = -r; j <= r; j += 1) {
            if (Math.round(Math.sqrt(i * i + j * j)) === r) {
                setPixelMatrix(i + cx, j + cy)
            }
        }
    }
}

function touchCoordinateToCellID(e, drawingArea) {
    //map (value, in_min, in_max, out_min, out_max)
    var targetCellX = parseInt(map(e.touches[0].clientX - drawingArea.offsetLeft, 0, window.innerWidth - (window.innerWidth * 0.1), 0, matrixSize)); //get x cell of touch
    var targetCellY = parseInt(map(e.touches[0].clientY - drawingArea.offsetTop, 0, window.innerWidth - (window.innerWidth * 0.1), 0, matrixSize)); //get y cell of touch

    if (targetCellX > 128) targetCellX = 128;
    if (targetCellY > 128) targetCellY = 128;
    if (targetCellX < 0) targetCellX = 0;
    if (targetCellY < 0) targetCellY = 0;

    if (drawingState) { //if drawing mode is on
        drawPen(document.getElementById('cell-' + (matrixSize * targetCellY + targetCellX))); //draw
    }
}
var drawingArea = document.getElementById('drawing-area');

function createMatrix() {

    drawingArea.style.width = window.innerWidth * 0.9 + 'px'; //set drawing area width (90% of window width)
    drawingArea.style.height = window.innerWidth * 0.9 + 'px'; //set drawing area height(90% of window width)

    var cellSize = parseInt(drawingArea.style.width) / matrixSize;

    for (var i = 0; i < matrixSize; i++) {

        var row = document.createElement('div');
        row.className = 'row';
        for (var j = 0; j < matrixSize; j++) {

            var cell = document.createElement('div');
            cell.className = 'cell';
            cell.id = 'cell-' + (matrixSize * i + j);

            cell.style.width = cellSize + 'px';
            cell.style.height = cellSize + 'px';


            cell.addEventListener('mousedown', function() { drawingState = true; });
            cell.addEventListener('touchstart', function(e) {
                drawingState = true;
                touchCoordinateToCellID(e, drawingArea); //e is the event
            });


            cell.addEventListener('mouseup', function() { drawingState = false; });
            cell.addEventListener('touchend', function() { drawingState = false; });

            cell.addEventListener('mousemove', function() {
                if (drawingState) {
                    drawPen(this);
                    sendPixel(this.id.replace('cell-', ''));
                }
            });
            cell.addEventListener('touchmove', function(e) {
                touchCoordinateToCellID(e, drawingArea); //e is the event
            });

            row.appendChild(cell);
        }
        drawingArea.appendChild(row);
    }
}
//createMatrix();



var drawingColor = '#FFFFFF';
var drawingSize = 5;


function drawPen(cell) {
    var cellID = cell.id.replace('cell-', '');
    // console.log(cellID);
    //draw a filled circle
    for (var i = 0; i < drawingSize; i++) {
        drawCircle(parseInt(cellID) % matrixSize, parseInt(cellID / matrixSize), i);
    }
    // draw a fill circle around the cell
}

function drawInMatrix(x, y) {
    for (var i = 0; i < drawingSize; i++) {
        drawCircle(x, y, i);
    }
}

const pickr = Pickr.create({
    el: '.color-picker',
    theme: 'nano', // or 'monolith', or 'nano'
    // container: '.profile-color-picker',
    showAlways: true,
    container: '.color-picker-container',
    appClass: 'color-picker-cutom',
    // useAsButton: true,
    //showAlways: true,
    swatches: [
        'rgb(244, 67, 54)',
        'rgb(233, 30, 99)',
        'rgb(156, 39, 176)',
        'rgb(103, 58, 183)',
        'rgb(63, 81, 181)',
        'rgb(33, 150, 243)',
        'rgb(3, 169, 244)',
        'rgb(0, 188, 212)',
        'rgb(0, 150, 136)',
        'rgb(76, 175, 80)',
        'rgb(139, 195, 74)',
        'rgb(0, 0, 0)',
        'rgb(255, 235, 59)',
        'rgb(255, 193, 7)'
    ],

    components: {

        // Main components
        // preview: true,
        // opacity: true,
        // hue: true,

        // Input / output Options
        interaction: {
            // hex: true,
            // rgba: true,
            // hsla: true,
            // hsva: true,
            // cmyk: true,
            // input: true,
            // clear: true,
            // save: true
        }
    }

});

pickr.on('change', (source, instance) => {
    pickr.setColor(source.toHEXA().toString());
});
pickr.on('changestop', (source, instance) => {
    pickr.setColor(instance._color.toHEXA().toString()); //jsp a quoi ca sert mais je sais que faut le mettre
    drawingColor = instance._color.toHEXA().toString(); //save the color
});
pickr.on('swatchselect', (source, instance) => {
    drawingColor = instance._color.toHEXA().toString(); //save the color
});

function clearDraw() {
    for (var i = 0; i < matrixSize; i++) {
        for (var j = 0; j < matrixSize; j++) {
            document.getElementById("cell-" + (matrixSize * i + j)).style.backgroundColor = '#000000';
        }
    }
}






//------------------------------------------------------------



// wait for the content of the window element
// to load, then performs the operations.
// This is considered best practice.
window.addEventListener('load', () => {

    resize(); // Resizes the canvas once the window loads
    document.getElementById("drawing-canvas").addEventListener('mousedown', startPainting);
    document.getElementById("drawing-canvas").addEventListener('touchstart', startPainting);

    document.getElementById("drawing-canvas").addEventListener('mouseup', stopPainting);
    document.getElementById("drawing-canvas").addEventListener('touchend', stopPainting);

    document.getElementById("drawing-canvas").addEventListener('mousemove', sketch);
    document.getElementById("drawing-canvas").addEventListener('touchmove', sketch);


    window.addEventListener('resize', resize);
});

const canvas = document.getElementById('drawing-canvas');

// Context for the canvas for 2 dimensional operations
const ctx = canvas.getContext('2d');

// Resizes the canvas to the available size of the window.
function resize() {

    if(window.innerWidth * 0.9 > 600)
        ctx.canvas.width = 600;
    else
        ctx.canvas.width = window.innerWidth * 0.9;
    
    if(window.innerWidth * 0.9 > 600)
        ctx.canvas.height = 600;
    else
        ctx.canvas.height = window.innerWidth * 0.9;
}

// Stores the initial position of the cursor
let coord = { x: 0, y: 0 };

// This is the flag that we are going to use to 
// trigger drawing
let paint = false;

// Updates the coordianates of the cursor when 
// an event e is triggered to the coordinates where 
// the said event is triggered.

function getPosition(event) {
    coord.x = (event.clientX || event.touches[0].clientX) - canvas.offsetLeft;
    coord.y = (event.clientY || event.touches[0].clientY) - canvas.offsetTop;

    // map(value, in_min, in_max, out_min, out_max)
    var targetCellX = parseInt(map(coord.x, 0, window.innerWidth - (window.innerWidth * 0.1), 0, matrixSize)); //get x cell of touch
    var targetCellY = parseInt(map(coord.y, 0, window.innerWidth - (window.innerWidth * 0.1), 0, matrixSize)); //get y cell of touch

    if (targetCellX > 128) targetCellX = 128;
    if (targetCellY > 128) targetCellY = 128;
    if (targetCellX < 0) targetCellX = 0;
    if (targetCellY < 0) targetCellY = 0;
    // drawPen(document.getElementById('cell-' + (matrixSize * targetCellY + targetCellX))); //draw

    // console.log("x: " + targetCellX + " y: " + targetCellY);


    drawInMatrix(targetCellX, targetCellY);
}

var drawing;

// The following functions toggle the flag to start
// and stop drawing
var sendDrawInterval;

function startPainting(event) {
    paint = true;
    drawing = true;
    getPosition(event);
    sketch(event);
    console.log("start");
    clearTimeout(drawTimeout);
    sendDrawInterval = window.setInterval(function() {
        send.dataBuild();
        if (!drawing) clearInterval(sendDrawInterval);

    }, 20);
}

var drawTimeout;

function stopPainting() {
    paint = false;
    clearInterval(sendDrawInterval);
    drawTimeout = window.setTimeout(function() { //au bout de 10 secondes, on depasse en mode gif
        drawing = false;
        console.log("stop");
        send.dataBuild();
        clearInterval(sendDrawInterval);
    }, 10000);
}

function clearMatrix() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    send.clearDraw();
}


function sketch(event) {

    if (!paint) return;

    ctx.beginPath();

    ctx.lineWidth = 20;

    // Sets the end of the lines drawn
    // to a round shape.
    ctx.lineCap = 'round';

    //set the color
    ctx.strokeStyle = drawingColor;

    // The cursor to start drawing
    // moves to this coordinate
    ctx.moveTo(coord.x, coord.y);

    // The position of the cursor
    // gets updated as we move the
    // mouse around.
    getPosition(event);

    // A line is traced from start
    // coordinate to this coordinate
    ctx.lineTo(coord.x, coord.y);

    // Draws the line.
    ctx.stroke();
}



