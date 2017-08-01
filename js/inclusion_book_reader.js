/**
 * Created by Benjamin Rosner (@blame github: br2490) on 7/17/2017.
 */

(function ($) {
    //	REPLACE, not modify, the existing 'willChangeToIndex' to silence the console output and then extend it further.
    BookReader.prototype.willChangeToIndex = function (index) {

        // "I don't know." - Ben Rosner, 2017.
        if (this.isCompound || this.onInclusion) {
            if (this.mode !== this.constMode1up) {
                console.log("These are best viewed in 1up mode. Switching your viewing mode now!");
                this.switchToolbarMode(1);
                this.switchMode(1);
            }
            this.inclusionHandler(index);
        }

        // Update navbar position icon - leads page change animation
        this.updateNavIndex(index);
    };

    // Does our bookreader window have the necessary elements to be a compound object?
    BookReader.prototype.onCompoundObject = function () {
        // Hang on to this, we'll need it.
        this.inclusionObjects = $("div.islandora-compound-thumb");
        this.onInclusionObjects = this.inclusionObjects.has(".active .inclusion-object");
        this.isCompound = this.inclusionObjects.length > 0;
        this.onInclusion = this.onInclusionObjects.length > 0;
        if (this.onInclusion) this.inclusionPageNumber = this.onInclusionObjects["0"].classList[3].match(/inclusion-page-(.*)/i)[1];
    };

    // canSwitchToMode overwrite.
    //________
    // Returns true if we can switch to the requested mode

    BookReader.prototype.canSwitchToMode = function (mode) {
        // compound objects are not allowed outside of 1up.
        if (this.isCompound || this.onInclusion) {
            return (mode === this.constMode1up);
        }

        if (mode == this.constMode2up || mode == this.constModeThumb) {
            // check there are enough pages to display
            // $$$ this is a workaround for the mis-feature that we can't display
            //     short books in 2up mode
            if (this.numLeafs < 2) {
                return false;
            }
        }
        return true;
    };

    // Handle inclusions for compound objects.
    BookReader.prototype.inclusionHandler = function (index) {
        var pageNumber, pageInclusions, returnUrl;

        if (this.onInclusion) {
            pageNumber = "inclusion-page-" + this.inclusionPageNumber;
        } else {
            pageNumber = "inclusion-page-" + this.getPageNum(index);
        }

        pageInclusions = this.inclusionObjects.has("." + pageNumber);

        // If we are on an inclusion, the parent no longer has the class .active. In this way when we are on an
        // inclusion we DO display the parent to return "home".
        this.inclusionObjects.has(".parent .active").hide();
        // Hide all but those that belong to this page, or this inclusion object.
        this.inclusionObjects.has(".inclusion-object").not(pageInclusions).hide();
        // Fade in if they're hidden.
        pageInclusions.has(":hidden").fadeIn();


        // Set a heading.
        if (this.onInclusion) {
            $(".islandora-compound-details").text(Drupal.t("Viewing an Inclusion of page " + this.inclusionPageNumber));
            returnUrl = this.inclusionObjects.has(".parent").find("a").attr("href");
            $("#return-to-page").html("<a name='return-to-page' href='" + returnUrl + "'>Back to Scrapbook page " + this.inclusionPageNumber + "</a>");
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
    };
})(jQuery);