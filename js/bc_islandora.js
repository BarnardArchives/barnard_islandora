/*global
    Drupal, jQuery, window
*/

(function ($) {
    Drupal.behaviors.bc_islandora = {
        // Newspaper (Barnard Bulletin) landing page.
        // Also defines some methods that run on yearbook landing page.
        bulletin_calendar: {
            show_calendar: function (lp_id) {
                var $selector = $('.' + lp_id + '-calendar');
                if ($selector.hasClass('inactive')) {
                    $selector.removeClass('inactive').addClass('active');
                }
            },
            // Hide child lists.
            hide_lists: function (lp_id) {
                var calendar_selector = '.' + lp_id + '-calendar ul';
                $(calendar_selector).each(function () {
                    if ($(this).parent().attr('class') !== 'decades') {
                        $(this).addClass('inactive');
                    } else {
                        $(this).addClass('active');
                    }
                });
            },
            listeners: function (lp_id) {
                var self = this;
                self.lp_class = '.' + lp_id + '-calendar';
                self.lp_nav = '.' + lp_id + '-nav';
                if (lp_id === 'bulletin') {
                    self.browse_phrase = 'Browse issues by date:';
                } else if (lp_id === 'yearbook') {
                    self.browse_phrase = 'Find a yearbook by date:';
                }
                // Decade click.
                $('#decades .decade a').click(function () {
                    var decade = $(this).html();
                    self.toggle_list('#decades', '.years #' + decade);
                    self.nav_display(self.lp_nav, '.decade', decade);
                    self.nav_display(self.lp_nav, '.browse', '<b><a href="#" id="nav">' + self.browse_phrase + '</a></b>&nbsp;');
                });
                // Year click.
                $('.year a').click(function () {
                    if (self.lp_nav === '.bulletin-nav') {
                        var year = $(this).parent().attr('id'),
                            decade = $(this).parent().parent().attr('id');
                        self.toggle_list('.years .decade', '.months-' + year);
                        self.nav_display(self.lp_nav, '.year', year);
                        self.nav_display(self.lp_nav, '.browse', '<a href="#" id="nav"><b>Browse issues by date:</b></a>&nbsp;');
                        self.nav_display(self.lp_nav, '.decade', '<a href="#" id="nav">' + decade + '</a>&nbsp;&gt;&nbsp;');
                    }
                });
                // Month click.
                $('.months li.month a').click(function () {
                    var year = $('.bulletin-nav .year').text(),
                        month = $(this).parent().attr('id'),
                        month_text = $(this).text(),
                        issues_selector = '.issues-' + year + '-' + month;
                    self.toggle_list('.months-' + year, issues_selector);
                    self.nav_display(self.lp_nav, '.month', month_text + ', ');
                    self.nav_display(self.lp_nav, '.year', '<a href="#">' + year + '</a>');
                });
                // Nav decade click.
                $('.bulletin-nav .decade').click(function () {
                    var decade = $(this).find('a').text(),
                        active_list = $(self.lp_class).find('ul.active'),
                        years_list = $(self.lp_class + ' .years ul.decade#' + decade);
                    if (decade.length !== 0) {
                        self.toggle_list(active_list, years_list);
                        self.nav_display('.bulletin-nav', '.year', '');
                        self.nav_display('.bulletin-nav', '.month', '');
                        self.nav_display('.bulletin-nav', '.decade', decade);
                    }
                });
                // Nav year click.
                $('.bulletin-nav .year').click(function () {
                    var active_list = $(self.lp_class).find('ul.active'),
                        month_list = $('ul.months-' + $(this).text());
                    self.toggle_list(active_list, month_list);
                    self.nav_display('.bulletin-nav', '.month', '');
                    self.nav_display('.bulletin-nav', '.year', $(self.lp_nav + ' .year').text());
                });
                // Nav browse click.
                $('.bulletin-nav .browse').click(function () {
                    var active_list = $(self.lp_class).find('ul.active');
                    self.toggle_list(active_list, $('#decades'));
                    $('#decades').removeClass('inactive').addClass('active');
                    self.nav_display('.bulletin-nav', '.year', '');
                    self.nav_display('.bulletin-nav', '.month', '');
                    self.nav_display('.bulletin-nav', '.browse', '<b>Browse issues by date:</b>&nbsp;');
                    self.nav_display('.bulletin-nav', '.decade', '');
                });
                // Yearbook nav browse click.
                $('.yearbook-nav .browse').click(function () {
                    var active_list = $(self.lp_class).find('ul.active');
                    self.toggle_list(active_list, $('#decades'));
                    self.nav_display('.yearbook-nav', '.browse', '<b>' + self.browse_phrase + '</b>&nbsp;');
                    self.nav_display('.yearbook-nav', '.decade', '');
                });
            },
            // Change the navigation text.
            nav_display: function (nav_class, selector, text) {
                $(nav_class).find(selector).html(text);
            },
            // Hide one selector and show another one instead.
            toggle_list: function (from_selector, to_selector) {
                $(from_selector).removeClass('active').addClass('inactive');
                $(to_selector).removeClass('inactive').addClass('active');
            },
            run: function () {
                if (window.location.pathname === '/bulletin' || window.location.pathname === '/yearbook') {
                    var lp_id = window.location.pathname.split('/')[1];
                    this.show_calendar(lp_id);
                    this.hide_lists(lp_id);
                    this.listeners(lp_id);
                }
            }
        },
        // Collections landing page.
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
        attach: function (context, settings) {
            this.bulletin_calendar.run();
            this.collections_lp.run();
            this.front_featured.run(settings);
        }
    };
}(jQuery));
