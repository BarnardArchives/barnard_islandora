(function ($) {
  $(document).ready(function () {
    var bulletin_calendar = {
        hide_lists: function () {
          $('.bulletin-calendar ul').each(function() {
            if ($(this).parent().attr('class') != 'years') {
              $(this).addClass('inactive');
            }
            else {
              $(this).addClass('active');
            }
          });
        },
        listeners: function () {
          var self = this;
          // year click
          $('.year a').click(function () {
            var year = $(this).parent().attr('id');
            var months = $('.months-' + year);
            self.toggle_list('.decade', '.months-' + year);
            self.nav_display('.year #nav', year);
            self.nav_display('.year', year);
            self.nav_display('.browse', '<a href="#" id="nav">Browse</a>&nbsp;&gt;&nbsp;');
          });
          // month click
          $('.months li.month a').click(function () {
            var year = $('.bulletin-nav .year').text();
            var month = $(this).parent().attr('id');
            var month_text = $(this).text();
            var issues_selector = '.issues-' + year + '-' + month;
            self.toggle_list('.months-' + year, issues_selector);
            self.nav_display('.month', month_text + ', ');
            self.nav_display('.year', '<a href="#">' + year + '</a>');
          });
          // nav year click
          $('.bulletin-nav .year').click(function () {
            var active_list = $('.bulletin-calendar').find('ul.active');
            var month_list = $('ul.months-' + $(this).text());
            self.toggle_list(active_list, month_list);
            self.nav_display('.month', '');
            self.nav_display('.year', $('.bulletin-nav .year').text());
          });
          // nav browse click
          $('.bulletin-nav .browse').click(function() {
            var active_list = $('.bulletin-calendar').find('ul.active');
            self.toggle_list(active_list, $('.years'));
            $('.decade').removeClass('inactive').addClass('active');
            self.nav_display('.year', '');
            self.nav_display('.month', '');
            self.nav_display('.browse', '');
          });
      },
      nav_display: function (selector, text) {
          $('.bulletin-nav').find(selector).html(text);
      },
      toggle_list: function (from_selector, to_selector) {
        $(from_selector).removeClass('active').addClass('inactive');
        $(to_selector).removeClass('inactive').addClass('active');
      },
      run: function () {
        this.hide_lists();
        this.listeners();
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
    if (window.location.href.indexOf('bulletin') !== -1) {
      bulletin_calendar.run();
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
