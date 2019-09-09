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
    // Modifies pagination per: 
    // Assumes bootstrap classes are used ".pagination > {.active, .active, .pager-first, pager-last}"
    paginationRedux: {
      run: function() {
        const islandoraPager = $('ul.pagination'), // Grab the pagination item and hold it.
            activePage = islandoraPager.find('li.active'), // active position
            firstPage = islandoraPager.find('li.pager-first > a'), // first item
            lastPage = islandoraPager.find('li.pager-last > a'), // last item
            lastPageNumber = lastPage.attr('href').match(/.*page=(\d*)/i)[1]; // greatest possible page number
         // lastPageNumber = $.urlParams(lastPage.attr('href')).page; // greatest possible page number

        if (activePage.text() > 4) {
          firstPage.text('1'); // set the first page text to the number 1.
          firstPage.parent().insertAfter(islandoraPager.find('li.prev')); // move the item forward
        } else {
          firstPage.hide(); // otherwise we don't need it
        }

        if (activePage.text() < (lastPageNumber - 2)) {
          lastPage.text(lastPageNumber); // set the last page to the last page number
          lastPage.parent().insertBefore(islandoraPager.find('li.next')); // move the item backward
        } else {
          lastPage.hide(); // otherwise we do not need it.
        }
      }
    },
    attach: function (context, settings) {
      this.collections_lp.run();
      this.front_featured.run(settings);
      this.paginationRedux.run();
    }
  };
}(jQuery));
