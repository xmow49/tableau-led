function updateLabel(input) {
    var labelId = input.id + '-label'; // id of the label
    var labelText = document.getElementById(labelId).innerHTML; // get the label text
    var unit = labelText.split(' ').pop(); //remove all text exept the last word (unit)
    document.getElementById(labelId).innerHTML = input.value + " " + unit; // update the label with the input value
}