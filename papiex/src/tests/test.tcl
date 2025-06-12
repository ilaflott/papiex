set domain [exec perl -T -e "use Net::Domain(hostdomain); print hostdomain"]; puts stderr $domain
