(function ($) {
  $(document).ready(function () {
    var bulletin_calendar = {
        hide_lists: function (lp_id) {
          var calendar_selector = '.' + lp_id + '-calendar ul';
          $(calendar_selector).each(function() {
            console.log(this);
            if ($(this).parent().attr('class') != 'decades') {
              $(this).addClass('inactive');
            }
            else {
              $(this).addClass('active');
            }
          });
        },
        listeners: function (lp_id) {
          console.log(lp_id);
          var self = this;
          self.lp_class = '.' + lp_id + '-calendar';
          self.lp_nav = '.' + lp_id + '-nav';
          if (lp_id == 'bulletin') {
            self.browse_phrase = 'Browse issues by date:';
          }
          else if (lp_id == 'yearbook') {
            self.browse_phrase = 'Find a yearbook by date:';
          }
          // decade click
          $('#decades .decade a').click(function() {
            var decade = $(this).html(),
              browse_phrase;
            console.log(decade);
            self.toggle_list('#decades', '.years #' + decade);
            self.nav_display(self.lp_nav, '.decade', decade);
            self.nav_display(self.lp_nav, '.browse', '<b><a href="#" id="nav">' + self.browse_phrase + '</a></b>&nbsp;');
          });
          // year click
          $('.year a').click(function () {
            if (self.lp_nav == '.bulletin-nav') {
              var year = $(this).parent().attr('id'),
                decade = $(this).parent().parent().attr('id'),
                months = $('.months-' + year);
              self.toggle_list('.years .decade', '.months-' + year);
              self.nav_display(self.lp_nav, '.year', year);
              self.nav_display(self.lp_nav, '.browse', '<a href="#" id="nav"><b>Browse issues by date:</b></a>&nbsp;');
              self.nav_display(self.lp_nav, '.decade', '<a href="#" id="nav">' + decade + '</a>&nbsp;&gt;&nbsp;');              
            }
          });
          // month click
          $('.months li.month a').click(function () {
            var year = $('.bulletin-nav .year').text(),
              month = $(this).parent().attr('id'),
              month_text = $(this).text(),
              issues_selector = '.issues-' + year + '-' + month;
            self.toggle_list('.months-' + year, issues_selector);
            self.nav_display(self.lp_nav, '.month', month_text + ', ');
            self.nav_display(self.lp_nav, '.year', '<a href="#">' + year + '</a>');
          });
          // nav decade click
          $('.bulletin-nav .decade').click(function() {
            var decade = $(this).find('a').text(),
              active_list = $(self.lp_class).find('ul.active'),
              years_list = $(self.lp_class + ' .years ul.decade#' + decade);
            self.toggle_list(active_list, years_list);
            self.nav_display('.bulletin-nav', '.year', '');
            self.nav_display('.bulletin-nav', '.decade', decade);
          });
          // nav year click
          $('.bulletin-nav .year').click(function () {
            var active_list = $(self.lp_class).find('ul.active'),
              month_list = $('ul.months-' + $(this).text());
            self.toggle_list(active_list, month_list);
            self.nav_display('.bulletin-nav', '.month', '');
            self.nav_display('.bulletin-nav', '.year', $(self.lp_nav + ' .year').text());
          });
          // nav browse click
          $('.bulletin-nav .browse').click(function() {
            var active_list = $(self.lp_class).find('ul.active');
            self.toggle_list(active_list, $('#decades'));
            $('#decades').removeClass('inactive').addClass('active');
            self.nav_display('.bulletin-nav', '.year', '');
            self.nav_display('.bulletin-nav', '.month', '');
            self.nav_display('.bulletin-nav', '.browse', '<b>Browse issues by date:</b>&nbsp;');
            self.nav_display('.bulletin-nav', '.decade', '');
          });
          $('.yearbook-nav .browse').click(function() {
            var active_list = $(self.lp_class).find('ul.active');
            self.toggle_list(active_list, $('#decades'));
            self.nav_display('.yearbook-nav', '.browse', '<b>' + self.browse_phrase + '</b>&nbsp;');
            self.nav_display('.yearbook-nav', '.decade', '');
          })
      },
      nav_display: function (nav_class, selector, text) {
        $(nav_class).find(selector).html(text);
      },
      toggle_list: function (from_selector, to_selector) {
        $(from_selector).removeClass('active').addClass('inactive');
        $(to_selector).removeClass('inactive').addClass('active');
      },
      run: function (lp_id) {
        this.hide_lists(lp_id);
        this.listeners(lp_id);
      }
    };
    
    var exhibition = {
      show_hide_all: function() {
        $('.exhibition-object').each(function() {
          $(this).css('left', '960px').addClass('inactive');
        })
        // $('.exhibition-object:first').removeClass('inactive').addClass('active');
        $('.exhibition-object:first').css('left', '0px').addClass('active');
      },
      listeners: function() {
        $('.exhibition-next a').click(function(event) {
          event.preventDefault();
          var $active = $('.exhibition').find('.active');
          var $next = $active.next();
          var $first = $('.exhibition').children().first();
          $active.removeClass('active').addClass('inactive');
          $active.css('left', '960px');
          if ($next.length != 0) {
            $next.removeClass('inactive').addClass('active');
            $next.css('left', '0px');
          }
          else {
            $first.removeClass('inactive').addClass('active');
            $first.css('left', '0px');
          }
        });
        $('.exhibition-prev a').click(function(event) {
          event.preventDefault();
          var $active = $('.exhibition').find('.active');
          var $prev = $active.prev();
          var $last = $('.exhibition').children().last();
          $active.removeClass('active').addClass('inactive');
          $active.css('left', '960px');
          if ($prev.length != 0) {
            $prev.removeClass('inactive').addClass('active');
            $prev.css('left', '0px');
          }
          else {
            $last.removeClass('inactive').addClass('active');
            $last.css('left', '0px');
          }
        });
      },
      run: function() {
        this.show_hide_all();
        this.listeners();
      }
    };

    // TODO (yuk)
    if (window.location.pathname == '/bulletin' || window.location.pathname == '/yearbook') {
      bulletin_calendar.run(window.location.pathname.split('/')[1]);
    }

    if (window.location.href.indexOf('exhibitions') !== -1) {
      exhibition.run();
    }
    
    // yuk!
    if (typeof Drupal.settings.featured_img_path !== 'undefined' && Drupal.settings.featured_img_path.length != 0) {
      $('body.front').css('background', 'url("' + Drupal.settings.featured_img_path + '") 50% 50% no-repeat fixed');
    }
  });
}) (jQuery);
