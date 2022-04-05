//c'est lancien system de dessin mais je le garde au cas ou







// wait for the content of the window element
// to load, then performs the operations.
// This is considered best practice.
window.addEventListener('load', () => {

    resize(); // Resizes the canvas once the window loads
    document.addEventListener('mousedown', startPainting);
    document.addEventListener('touchstart', startPainting);

    document.addEventListener('mouseup', stopPainting);
    document.addEventListener('touchend', stopPainting);

    document.addEventListener('mousemove', sketch);
    document.addEventListener('touchmove', sketch);


    window.addEventListener('resize', resize);
});

const canvas = document.getElementById('drawing-canvas');

// Context for the canvas for 2 dimensional operations
const ctx = canvas.getContext('2d');

// Resizes the canvas to the available size of the window.
function resize() {
    ctx.canvas.width = window.innerWidth * 0.9;
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
    drawInMatrix(targetCellX, targetCellY);
}

// The following functions toggle the flag to start
// and stop drawing
function startPainting(event) {

    paint = true;
    getPosition(event);
    sketch(event);
}

function stopPainting() {
    paint = false;
}

function sketch(event) {
    if (!paint) return;
    ctx.beginPath();

    ctx.lineWidth = 25;

    // Sets the end of the lines drawn
    // to a round shape.
    ctx.lineCap = 'round';

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