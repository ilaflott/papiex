set domain [exec perl -e "use Net::Domain(hostdomain); print hostdomain"]; puts stderr $domain
