<!DOCTYPE html>
<html lang="fr">
    <head>

    </head>
    <body>
        <div>
           <form action="post.php" method="POST">
               <input type="text" name="gif">
               <button type="submit">Envoyer</button> 
           </form>
           <?php 
           echo print_r($_POST);

           $file = './matrix';

            // Ouvre un fichier pour lire un contenu existant
            $current = file_get_contents($file);
            echo $current;

            $data = explode(",",implode($_POST));

            if($data[0] == "GIF"){
                $current = "TO=CPP"."\n"."MODE=".$data[0]."\n"."GIF=". $data[1]."\n"."SPEED=".$data[2];
            }
            else if($data[0] == "DRAW"){
                $current = "TO=CPP"."\n"."MODE=".$data[0]."\n";
                for($i = 1; $i < count($data); $i++){
                    $current .= 'L'.$data[$i]."\n";
                }
                // $matrixArray=explode(",",implode($_POST));
                
            }
            
            echo $current;

            // Écrit le résultat dans le fichier
            file_put_contents($file, $current);

           ?>
        </div>
    </body>
</html>
