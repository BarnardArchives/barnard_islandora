/**
 * Created by Benjamin Rosner (@blame github: br2490) on 7/17/2017.
 */
(function ($) {
    // Does our bookreader window have the necessary elements to be a compound object?
    BookReader.prototype.compoundInit = function () {
        this.inclusionObjects = $("div.islandora-compound-thumb");
        this.isOnCompound = function() { return this.inclusionObjects.length > 0; };

        if (!this.isOnCompound) {
            return;
        }

        this.activeInclusionObjects = this.inclusionObjects.has(".active .inclusion-object");
        this.isOnInclusion = this.activeInclusionObjects.length > 0;

        // Remove the parent object TN entirely from view while viewing it in the BookReader.
        this.inclusionObjects.has(".parent").hide();

        if (this.isOnInclusion) {
            $(".islandora-compound-details").hide();
            // Pull our inclusion-page identifier from the classes that wrap them.
            this.inclusionPageNumber = this.activeInclusionObjects["0"].classList[3].match(/inclusion-page-(.*)/i)[1];
            this.activeInclusionDiv = "inclusion-page-" + this.inclusionPageNumber;
            var parentAlumBreadcrumb = $("div.breadcrumb a:last");
            this.returnUrl = parentAlumBreadcrumb.attr('href') + '#page/' + this.inclusionPageNumber + '/mode/1up';
            parentAlumBreadcrumb.attr('href', this.returnUrl);
            $('.islandora-compound-thumb a.active').attr('href', this.returnUrl);
        }
    };

    BookReader.prototype.switchToCompoundObjectDisplay = function () {
        $("#BRnav").find(".onepg, .twopg, .thumb").hide();
        // each icon has a width of 40px, and the navpos has an initial width of 280px... this could be mathed, and done
        // more appropriately, but keeping in the spirit.
        $("#BRnavpos").css({'margin-right': 160});
        $("#BRpage").css({'width': 160});

        // Block reposition
        var reader = $("#BookReader");
        var block = $("#block-islandora-compound-object-compound-navigation");
        block.offset({'top': reader.offset().top, 'left': block.offset().left});
    };

    // Handle inclusions for compound objects.
    BookReader.prototype.inclusionDisplayHandler = function (index) {
        var inclusionPageDiv, pageInclusions;

        if (this.isOnInclusion) {
            inclusionPageDiv = this.activeInclusionDiv;
        } else {
            inclusionPageDiv = "inclusion-page-" + this.getPageNum(index);
        }

        pageInclusions = this.inclusionObjects.has("." + inclusionPageDiv);
        // Fade in if they have been hidden previously.
        pageInclusions.has(":hidden").show();
        // Hide all but those that belong to this page or inclusion object.
        this.inclusionObjects.has(".inclusion-object").not(pageInclusions).hide();

        // Set a heading.
        if (this.isOnInclusion) {
            $("#return-to-page").html("<a name='return-to-page' href='" + this.returnUrl + "'>Back to scrapbook page " + this.inclusionPageNumber + "</a>");
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

    /*
     * Put handlers here for when we will navigate to a new page
     */
    BookReader.prototype.willChangeToIndex = function (index) {
        if (this.isOnCompound) this.inclusionDisplayHandler(index);

        // Update navbar position icon - leads page change animation
        this.updateNavIndex(index);
    };

    // Returns true if we can switch to the requested mode
    BookReader.prototype.canSwitchToMode = function (mode) {
        if (this.isOnCompound) {
            // Compound objects are not allowed outside of 1up view.
            if (this.mode !== this.constMode1up) this.mode = this.constMode1up;
            return (mode === this.constMode1up);
        }
        if (mode == this.constMode2up || mode == this.constModeThumb) {
            // check there are enough pages to display
            // $$$ this is a workaround for the mis-feature that we can't display
            // short books in 2up mode
            if (this.numLeafs < 2) {
                return false;
            }
        }
        return true;
    };

})(jQuery);