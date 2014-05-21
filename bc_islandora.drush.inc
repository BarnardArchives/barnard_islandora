<?php

require_once 'includes/bc_islandora.drush_helpers.inc';

function bc_islandora_drush_command() {
  return array(
    'bc_islandora_xml_export' => array(
      'description' => 'Exports an FOXML representation of the given object.',
      'arguments' => array(
        'pid' => 'The pid of the object to export.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost xml_export islandora:37',
      ),
      'aliases' => array('bcixe'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    ),
    'bc_islandora_purge_object' => array(
      'description' => 'Purges an object from the repository.',
      'arguments' => array(
        'pids' => 'A comma-separated list of unique identifiers.',
      ),
      'aliases' => array('bcipo'),
    ),
    'bc_islandora_ingest_newspapers' => array(
      'description' => "Creates the parent issue object and uploads the pages as child objects.  
        Requires the Drupal7 version of Islandora with tuque",
      'arguments' => array(
        'query_file' => 'The path to a CSV file that contains the information about the newspaper pages',
      ),
      'options' => array(
        'derivatives' => 'Whether to create derivatives.',
        'overwrite-existing' => 'Whether to overwrite existing objects.',
        // TODO move this
        'purge' => 'Whether to purge instead of ingesting.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost ingest_newspapers /path/to/pages.csv',
      ),
      'aliases' => array('bciin'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
    ),
    'bc_islandora_ingest_books' => array(
      'description' => 'Ingests books.',
      'arguments' => array(
        'query_file' => 'A csv file containing information about the books to be ingested.'
      ),
      'options' => array(
        'derivatives' => 'Whether to create derivatives.',
        'overwrite-existing' => 'Whether to overwrite existing objects.',
        // TODO move this
        'purge' => 'Whether to purge instead of ingesting.',
        'year' => 'Whether to import a specific year\'s book.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost bciib /path/to/pages.csv',
      ),
      'aliases' => array('bciib'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
    ),
    'bc_islandora_generate_newspaper_csv' => array(
      'description' => 'Generates csv files for ingesting newspapers.',
      'arguments' => array(
        'directory' => 'The directory containing image files.',
      ),
      'options' => array(
        'year' => 'The year for which to generate csv.',
        'month' => 'The month for which to generate csv.',
      ),
      'examples' => array(
        'drush generate_newspaper_csv /vagrant/bulletin',
      ),
      'aliases' => array('bcignc'),
    ),
  );
}

function drush_bc_islandora_xml_export($pid) {
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $foxml = $repository->api->m->export($pid, array(
    'format' => 'info:fedora/fedora-system:FOXML-1.1',
    'context' => 'migrate',
    'encoding' => 'UTF-8',
  ));
  var_dump($foxml);
}

function drush_bc_islandora_ingest_newspapers($query_file) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');

  ini_set("auto_detect_line_endings", "1");

  $path_name = dirname($query_file);
  $fh = fopen($query_file, 'r');
  $line_number = 0;
  $issues = array();

  while (($data = fgetcsv($fh)) !== FALSE) {
    $line_number++;
    if ($line_number > 1) {
      $type = $data[4];
      if ($type == 'issue') {
        $issue_pid = 'bc:bulletin-' . date('Ymd', strtotime($data[1]));
  
        if (drush_get_option('purge')) {
          drush_bc_islandora_purge_object($issue_pid);
        }
        elseif (!islandora_object_load($issue_pid)) {
          try {
            $issue_pid = create_issue_object($issue_pid, $data);
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        elseif (drush_get_option('overwrite-existing')) {
          try {
            drush_bc_islandora_purge_object($issue_pid);
            create_issue_object($issue_pid, $data);
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        else {
          drush_log(dt('Issue exists at pid = @pid; skipping...', array('@pid' => $issue_pid)), 'warning');
        }
        $issues[] = $issue_pid;
      }
      elseif ($type == 'page') {
        $page_pid = 'bc:bulletin-' . date('Ymd', strtotime($data[1])) . '-' . $data[3];
        
        if (drush_get_option('purge')) {
          drush_bc_islandora_purge_object($issue_pid);
        }
        elseif (!islandora_object_load($page_pid)) {
          try {
            create_page_object($path_name, $data, $issue_pid, $page_pid, 'newspaper');
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        elseif (drush_get_option('purge')) {
          drush_bc_islandora_purge_object($page_pid);
        }
        elseif (drush_get_option('overwrite-existing')) {
          try {
            drush_bc_islandora_purge_object($page_pid);
            create_page_object($path_name, $data, $issue_pid, $page_pid, 'newspaper');
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        else {
          drush_log(dt('Page exists at pid = @pid; skipping...', array('@pid' => $page_pid)), 'warning');
        }
      }
    }
  }    

  // Create derivatives.
  if (drush_get_option('derivatives')) {
    _bc_islandora_create_derivatives($issues);
  }
}

function drush_bc_islandora_ingest_books($query_file) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');

  ini_set("auto_detect_line_endings", "1");
  
  $fh = fopen($query_file, 'r');
  $year = drush_get_option('year');
  $books = array();

  while (($data = fgetcsv($fh)) !== FALSE) {
    $csv_year = array_pop(explode(' ', $data[1]));
    if ($year && $year != $csv_year) {
      continue;
    }
    $book_pid = 'bc:yearbook-' . $csv_year;
    $pages = $data[12];
    if (drush_get_option('purge')) {
      drush_bc_islandora_purge_object($book_pid);
    }
    elseif (!islandora_object_load($book_pid)) {
      try {
        print "Got book pid: $book_pid\n";
        $book_pid = create_book_object($book_pid, $data);
      }
      catch (Exception $e) {
        drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
      }
    }
    elseif (drush_get_option('overwrite-existing')) {
      try {
        drush_bc_islandora_purge_object($book_pid);
        create_book_object($book_pid, $data);
      }
      catch (Exception $e) {
        drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
      }
    }
    else {
      drush_log(dt('Issue exists at pid = @pid; skipping...', array('@pid' => $book_pid)), 'warning');
      continue;
    }
    $books[] = $book_pid;

    // Create each page.
    $file_path = dirname($query_file) . '/' . $csv_year;
    if (file_exists($file_path)) {
      $files = explode("\n", `ls $file_path/*.jp2`);
      foreach ($files as $fn) {
        if (!empty($fn)) {
          $page_number = preg_replace('/^0+/', '', preg_replace('/\.jp2$/', '', array_pop(explode('_', $fn))));
          $data[] = $page_number;
          // var_export($page_number);
          $page_pid = $book_pid . '-' . $page_number;
          if (drush_get_option('overwrite-existing') || drush_get_option('purge')) {
            try {
              drush_bc_islandora_purge_object($page_pid);
            }
            catch (Exception $e) {
              drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
            }
          }
          if (!drush_get_option('purge')) {
            create_page_object($fn, $data, $book_pid, $page_pid, 'book'); 
          }
        }        
      }
    }
  
    // Create derivatives.
    if (drush_get_option('derivatives')) {
      _bc_islandora_create_derivatives($books);
    }
  }
}

function drush_bc_islandora_purge_object($pids) {
  $tuque = new IslandoraTuque();
  foreach (explode(',', $pids) as $pid) {
    drush_log(dt('Purging object at pid = @pid...', array('@pid' => $pid)), 'ok');
    try {
      $tuque->repository->purgeObject($pid);
    }
    catch (Exception $e) {
      drush_log('Caught exception: @e', array('@e' => $e->getMessage()), 'warning');
    }
  }
}

function drush_bc_islandora_generate_newspaper_csv($directory = '/home/BCIT/dsavage/bulletin') {
  $year = drush_get_option('year');
  $month = drush_get_option('month');
  $filename = $year && $month ? "$year-$month" : ($year ? "$year" : '');
  $filename .= 'pages.csv';
  $csv = fopen("$directory/$filename", 'w');
  $lines = array(
    array("title", "date", "filename", "page number", "type", "extent"),
  );
  $issues = array();

  foreach (scandir($directory) as $file) {
    if (mime_content_type("$directory/$file") == 'image/tiff') {
      $ts = strtotime(array_shift(explode('-', str_replace('BarnardBulletin-', '', $file))));
      $day = date('d', $ts);
      if ((!$month || ($month && date('m', $ts) == $month)) && (!$year || ($year && date('Y', $ts) == $year))) {
        $extent_cmd = 'ls ' . $directory . '/BarnardBulletin-' . date('Ymd', $ts) . '*.tiff | wc -l';
        $extent = trim(`$extent_cmd`);
        $page = preg_replace('/^0+/', '', str_replace('.tiff', '', end(explode('-', $file))));
        $date_title = date('F j, Y', $ts);
        if ($page == 1) {
          $lines[] = array("Barnard Bulletin, " . $date_title, $date_title, '', '', 'issue', $extent);
        }
        $lines[] = array($date_title, $date_title, $file, $page, 'page', $extent);
      }
    }
  }
  foreach ($lines as $line) {
    fputcsv($csv, $line);
  }
  fclose($csv);
}

function _bc_islandora_create_derivatives($objects) {
  foreach ($objects as $pid) {
    $book = islandora_object_load($pid);
    $pages = array_keys(islandora_paged_content_get_pages($book));
    $img_batch = islandora_paged_content_create_images_batch($book, $pages);
    $pdf_batch = islandora_paged_content_create_pdf_batch($book, $pages);
    $ocr_batch = islandora_paged_content_create_ocr_batch($book, $pages, array('language' => 'eng', 'preprocess' => FALSE));
    batch_set($img_batch);
    batch_set($pdf_batch);
    batch_set($ocr_batch);
  }
  drush_backend_batch_process();
}