#!/bin/bash
# Indique au système que l'argument qui suit est le programme utilisé pour exéc$
# En règle générale, les "#" servent à mettre en commentaire le texte qui suit $
echo Changement mode COMFORT
exec 3<>/dev/tcp/"127.0.0.1"/51717
echo -e "CONF\n" >&3
exit 0
