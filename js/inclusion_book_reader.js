/**
 * Created by Benjamin Rosner (@blame github: br2490) on 7/17/2017.
 */

(function ($) {
//	REPLACE, not modify, the existing 'willChangeToIndex' to silence the console output and then extend it further.
    BookReader.prototype.willChangeToIndex = function (index) {
        // Update navbar position icon - leads page change animation
        // console.log(update nav index: " + index);
        this.updateNavIndex(index);
        // This is actually really poor code. I should be determining if this is a compound object when during
        // bookreader init, storing the inclusions and creating a mapping function called here, not checking at every
        // willChangeToIndex if this is a compound object. Probably? I don't know. -Ben Rosner, 2017.
        if ((inclusionObjects = $("div.islandora-compound-thumb")).length > 0) {
            console.log("Inclusion handler.");
            this.inclusionHandler(index, inclusionObjects);
        }
    }

    // Handle inclusions for compound objects. Pass in the current index and jQuery obj containing matches.
    BookReader.prototype.inclusionHandler = function (index, inclusionObjects) {

        // Hide all.
        inclusionObjects.has(".parent .active").hide();
        inclusionObjects.has(".inclusion-object").hide();

        var isInclusion = inclusionObjects.has('.active .inclusion-object');

        // Page Number - DIV
        if (isInclusion.length > 0) {
            var pageNumber = isInclusion["0"].classList[3];
        } else {
            var pageNumber = "inclusion-page-" + this.getPageNum(index);
        }

        // jQuery Object containing all inclusions from the current pageNumber.
        var pageInclusions = inclusionObjects.has("." + pageNumber);

        // Set a heading.
        if (isInclusion.length > 0) {
            $(".islandora-compound-details").text(Drupal.t("Viewing an Inclusion of"));
        } else {
            if (pageInclusions.length > 0) {
                $(".islandora-compound-details").text(Drupal.t("@Inclusion for page @page", {
                    '@Inclusion': Drupal.formatPlural(pageInclusions.length, 'Inclusion', 'Inclusions'),
                    '@page': this.getPageNum(index)
                }));
            } else {
                $(".islandora-compound-details").text(Drupal.t("No inclusions for page @page", {
                    '@page': this.getPageNum(index)
                }));
            }
        }

        // Show ALL inclusions for the current page only. All other inclusions remain hidden awaiting their turn.
        pageInclusions.fadeIn();
    }
})(jQuery);