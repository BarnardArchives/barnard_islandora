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
        // Hide all but the active and parent objects.
        inclusionObjects.not(".parent").not(".active").hide();
        // Show inclusions for this page only. All others remain hidden.
        inclusionObjects.has(".inclusion_object_page_" + (parseInt(index) + 1)).fadeIn();
    }
})(jQuery);