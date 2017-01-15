#!/bin/bash
# Indique au système que l'argument qui suit est le programme utilisé pour exécuter ce fichier
# En règle générale, les "#" servent à mettre en commentaire le texte qui suit comme ici
echo Changement DEBUG Mode

exec 3<>/dev/tcp/"127.0.0.1"/51717
echo -e "$1\n" >&3
exit 0


