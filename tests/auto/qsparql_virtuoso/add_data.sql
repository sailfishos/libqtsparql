sparql
prefix nco: <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#>
prefix nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#>
insert into <http://www.openlinksw.com/schemas/virtrdf#>

{
    <qsparql-virtuoso-tests> a nie:InformationElement .

    <uri001> a nco:PersonContact, nie:InformationElement ;
        nie:isLogicalPartOf <qsparql-virtuoso-tests> ;
        nco:nameGiven "name001" .

    <uri002> a nco:PersonContact, nie:InformationElement ;
        nie:isLogicalPartOf <qsparql-virtuoso-tests> ;
        nco:nameGiven "name002" .

    <uri003> a nco:PersonContact, nie:InformationElement ;
        nie:isLogicalPartOf <qsparql-virtuoso-tests> ;
        nco:nameGiven "name003" .
};




