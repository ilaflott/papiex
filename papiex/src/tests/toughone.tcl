set domain [exec /usr/bin/perl -e "use Net::Domain(hostdomain); print hostdomain"]; puts stderr $domain
