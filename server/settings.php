<!DOCTYPE html>
<html lang="fr">

<head>
    <link rel='stylesheet' href='css/index.css'>
    <link rel="stylesheet" href="dl/mdi/mdi.css" />
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0">

</head>

<body>

    <header>
        <h1>Paramètre</h1>
        <a href="index.php"><button><span class="mdi mdi-close"></span></button></a>
    </header>

    <div class="option-container">

        <div class="option-item">
            <h2>Distance de détection</h2>
            <input type="range" id="sensor-distance" min="2" max="400" value="200" oninput="updateLabel(this)">
            <label for="sensor-distance" id="sensor-distance-label">200 cm</label>
        </div>

        <div class="option-item">
            <h2>Vitesse d'animation</h2>
            <input type="range" id="animation-speed" min="50" max="150" value="100" oninput="updateLabel(this)">
            <label for="animation-speed" id="animation-speed-label">100 %</label>
        </div>

        <div class="option-item ">
            <h2>Sens de rotation</h2>
            <select name="animation-list " id="animation-style ">
                <option value="1 ">Rotation à gauche</option>
                <option value="2 ">Rotation à droite</option>
            </select>
        </div>

        <div class="option-item ">
            <h2>Style d'animation</h2>
            <select name="animation-list " id="animation-style ">
                <option value="0 ">Style 1</option>
                <option value="1 ">Style 2</option>
                <option value="2 ">Style 3</option>
                <option value="3 ">Style 4</option>
            </select>
        </div>
    </div>

</body>
<script src="js/settings.js "></script>

</html>