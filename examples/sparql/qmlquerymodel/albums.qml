import Qt 4.7
import Test 1.0

ListView {
    width: 1000; height: 600

    model:
        SparqlModel {
            hostName: "dbpedia.org"
            query: "SELECT ?predicate ?thing WHERE { <http://dbpedia.org/resource/The_Beatles> ?predicate ?thing . }"
        }

    delegate: Text { text: "Data: " + predicate + ", " + thing }
}
