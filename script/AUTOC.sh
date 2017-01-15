#!/bin/bash
# Indique au système que l'argument qui suit est le programme utilisé pour exécuter ce fichier
# En règle générale, les "#" servent à mettre en commentaire le texte qui suit comme ici
echo Changement mode AUTO submode CONF
exec 3<>/dev/tcp/"127.0.0.1"/51717
echo -e "AUTOC\n" >&3
exit 0


