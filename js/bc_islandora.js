/*global
    Drupal, jQuery, window
*/

(function ($) {
  Drupal.behaviors.bc_islandora = {
    collections_lp: {
      listeners: function () {
        $('.landingpage-icon').click(function () {
          window.location = $(this).find('a').attr('href');
        });
      },
      run: function () {
        if (window.location.pathname === '/collections') {
          this.listeners();
        }
      }
    },
    // Front page feature.
    // Assumes that settings.featured_img_path will be set (via backend).
    front_featured: {
      run: function (settings) {
        if (settings.featured_img_path !== undefined && settings.featured_img_path.length !== 0) {
          $('body.front').css('background', 'url("' + settings.featured_img_path + '") 50% 50% no-repeat fixed');
        }
      }
    },
    // Modifies pagination per: https://github.com/BarnardArchives/digitalcollections.barnard.edu/issues/74
    // Assumes bootstrap classes are used ".pagination > {.active, .pager-first, pager-last}"
    paginationRedux: {
      run: function() {
        const islandoraPager = $('ul.pagination'), // Grab the pagination item and hold it.
            activePage = islandoraPager.find('li.active'), // active position
            firstPage = islandoraPager.find('li.pager-first > a'), // first item
            lastPage = islandoraPager.find('li.pager-last > a'), // last item
            lastPageNumber = lastPage.length ? lastPage.attr('href').match(/.*page=(\d*)/i)[1] : 0; // greatest possible page number
        // lastPageNumber = $.urlParams(lastPage.attr('href')).page; // greatest possible page number - this requires a newer version of jQuery!


        if (activePage.text() > 3) {
          firstPage.text('1'); // set the first page text to the number 1.
          firstPage.parent().insertAfter(islandoraPager.find('li.prev')); // move the item forward
        } else {
          firstPage.hide(); // otherwise we don't need it
        }

        if (activePage.text() === '4') {
          firstPage.parent().next().hide();
        }

        if (activePage.text() < (lastPageNumber - 1)) {
          lastPage.text(Number(lastPageNumber) + 1); // set the last page to the last page number
          lastPage.parent().insertBefore(islandoraPager.find('li.next')); // move the item backward
        } else {
          lastPage.hide(); // otherwise we do not need it.
        }
      }
    },
    // Modifies search result heading per: https://github.com/BarnardArchives/digitalcollections.barnard.edu/issues/75
    solrSearchResultHeading: {
      run: function () {
        if ($('.node-type-islandora-solr-content-type').length) return;

        const pageHeading = $('h1.page-header'),
            islandoraResultCount = $('div#islandora-solr-result-count');

        if (islandoraResultCount.length && pageHeading.length) {
          const resultCount = islandoraResultCount.text().match(/.*of.(\d*)/i)[1],
              spanResultCount = $( '<span />' , {
                id: 'title-bound-solr-result-count',
                text: resultCount + ' ', // I don't like this, the space should be added elsewhere -BR
                style: 'font-weight: 900', // this should be removed and done in CSS. -BR
              });
          pageHeading.prepend(spanResultCount);
          islandoraResultCount.hide();
        }
      }
    },
    attach: function (context, settings) {
      this.collections_lp.run();
      this.front_featured.run(settings);
      this.paginationRedux.run();
      this.solrSearchResultHeading.run();
    }
  };
}(jQuery));
