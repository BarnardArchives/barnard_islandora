/**
 * @file
 * Defines initializing/attaching the Book Reader to the
 * defined element.
 */

(function ($) {
    Drupal.behaviors.islandoraInternetArchiveBookReader = {
        attach: function (context, settings) {
            $('.barnard--islandora-internet-archive-bookreader', context).once('islandora-bookreader', function () {
                var bookReader = new IslandoraBookReader(settings.islandoraInternetArchiveBookReader);
                // Sanity operations for compound objects.
                bookReader.compoundInit();
                // Initialize and Render the BookReader.
                bookReader.init();

                if (bookReader.isOnCompound()) {
                    bookReader.switchToCompoundObjectDisplay();
                }

                // Hide unused or unwanted toolbar buttons from view.
                $('#BRtoolbar').find('.read, .info, .share, .play, .pause').hide();

                // fixes odd zooming issues on large resolution inclusions.
                // if (bookReader.isOnInclusion) {
                //     // bookReader.zoom(-1);
                // }

                if (!bookReader.searchEnabled()) {
                    $('#textSrch').hide();
                    $('#btnSrch').hide();
                }

                // Handle page resize, required for full screen.
                $(window).resize(function () {
                    bookReader.windowResize();
                });

                if ($.browser.mobile && settings.islandoraInternetArchiveBookReader.mobilize) {
                    bookReader.goFullScreen();
                }
            });
        }
    };
})(jQuery);
