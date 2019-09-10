/*global
    Drupal, jQuery
*/

(function ($) {
  // Modifies search result heading per: https://github.com/BarnardArchives/digitalcollections.barnard.edu/issues/75
  Drupal.behaviors.solrSearchResultHeading = {
    attach: function (context, settings) {

      const pageHeading = $('h1.page-header'),
          islandoraResultCount = $('div#islandora-solr-result-count');

      islandoraResultCount.hide(); // I could hook the appropriate code in an alter but no thanks.

      if (typeof settings.islandoraSolrContent !== 'undefined' &&
          settings.islandoraSolrContent.userSearch === false ) return;

      if (islandoraResultCount.length && pageHeading.length) {
        const resultCount = islandoraResultCount.text().match(/.*of.(\d*)/i)[1],
            spanResultCount = $('<span />', {
              id: 'title-bound-solr-result-count',
              text: resultCount + ' ', // I don't like this, the space should be added elsewhere -BR
              style: 'font-weight: 900', // this should be removed and done in CSS. -BR
            });
        pageHeading.prepend(spanResultCount);
      }
    }
  };

})( jQuery );