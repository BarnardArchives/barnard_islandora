<?php

function create_issue_object($pid, $data) {
  module_load_include('inc', 'islandora', 'RestConnection');
  
  $tuque = islandora_get_tuque_connection();
  $connection = $tuque->connection;
  
  $title = $data[0];
  $date = date('Y-m-d', strtotime($data[1]));
  $date_short = date('Ymd', strtotime($date));
  $extent = $data[5];
 
  $dsid = 'MODS';

  $content_model_pid = 'islandora:newspaperIssueCModel';

  // TODO should be able to supply this as an argument. (or via CSV?)
  $collection_pid = variable_get('bc_islandora_newspaper_pid', 'islandora:74');

  $xml = <<<ISSUE_MODS
    <mods xmlns="http://www.loc.gov/mods/v3" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.loc.gov/mods/v3 http://www.loc.gov/mods/v3/mods-3-5.xsd">
      <titleInfo>
        <title>$title</title>
      </titleInfo>
      <genre authority="marcgt">newspaper</genre>
      <physicalDescription>
        <form authority="marcform">print</form>
        <form authority="marcform">electronic</form>
        <extent>$extent pages</extent>
        <internetMediaType>image/tiff</internetMediaType>
        <digitalOrigin>reformatted digital</digitalOrigin>
      </physicalDescription>
      <relatedItem type="host"> 
        <titleInfo> 
          <title>Barnard Bulletin</title> 
        </titleInfo> 
        <typeOfResource>text</typeOfResource> 
        <identifier type="local">BC12-03</identifier> 
      </relatedItem> 
      <originInfo>
        <dateIssued encoding="w3cdtf">$date</dateIssued>
        <dateCreated encoding="w3cdtf">$date</dateCreated>
      </originInfo>
      <identifier type="local">BC12-03_$date_short</identifier>
    </mods> 
ISSUE_MODS;
  
  $rdf_string = <<<ISSUE_RDF
    <rdf:RDF xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:islandora="http://islandora.ca/ontology/relsext#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
      <rdf:Description rdf:about="info:fedora/$pid">
        <fedora-model:hasModel rdf:resource="info:fedora/$content_model_pid"></fedora-model:hasModel>
        <fedora:isMemberOf rdf:resource="info:fedora/$collection_pid"></fedora:isMemberOf>
        <islandora:isSequenceNumber>1</islandora:isSequenceNumber>
        <islandora:dateIssued>$date</islandora:dateIssued>
      </rdf:Description>
    </rdf:RDF>
ISSUE_RDF;
 
  $mods_xml = new DOMDocument();
  $mods_xml->loadXML($xml);

  $transform_xsl = new DOMDocument();
  $xsl_file = DRUPAL_ROOT . '/' . drupal_get_path('module', 'islandora_importer') . '/xsl/mods_to_dc.xsl';
  $transform_xsl->load($xsl_file);

  $proc = new XSLTProcessor();
  $proc->importStylesheet($transform_xsl);
  $dc = $proc->transformToXML($mods_xml);

  $object = new NewFedoraObject($pid, $connection->repository);
  
  $object->label = $title;
 
  $dc_datastream = new NewFedoraDatastream('DC', 'X', $object, $connection->repository);

  $dc_datastream->content = $dc;
  $dc_datastream->mimetype = 'text/xml';
  $dc_datastream->label = 'Dublin Core Record for this object';

  $object->ingestDatastream($dc_datastream);

  $mods_datastream = new NewFedoraDatastream('MODS', 'X', $object, $connection->repository);
  
  $mods_datastream->setContentFromString($xml);
  $mods_datastream->mimetype = 'text/xml';
  $mods_datastream->label = 'MODS record';
  $mods_datastream->checksum = TRUE;
  $mods_datastream->checksumType = 'MD5';
  $mods_datastream->logMessage = 'MODS datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($mods_datastream);
  
  $rels_datastream = new NewFedoraDatastream('RELS-EXT', 'X', $object, $connection->repository);
  $rels_datastream->setContentFromString($rdf_string);
  $rels_datastream->label = 'RELS-EXT';
  $rels_datastream->mimetype = 'text/xml';
  $rels_datastream->logMessage = 'RELS-EXT datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($rels_datastream);  
  try {
    $object->state = 'A';
    $ingest = $tuque->repository->ingestObject($object);
  }
  catch(Exception $e) {
    unset($object);
    throw($e);
  }

  if ($ingest == TRUE) {
    drush_log(dt("Book object @pid created successfully\n", array('@pid' => $pid)), 'success');
  }
  else {
    drush_log(dt("There was an error creating the book object!\n", 'warning'));
  }
  unset($object);
  unset($mods_datastream);
  unset($rels_datastream);
  unset($dc_datastream);
  
  return $pid;
}

function create_book_object($pid, $data) {
  module_load_include('inc', 'islandora', 'RestConnection');
  
  $tuque = islandora_get_tuque_connection();
  $connection = $tuque->connection;

  $dsid = 'MODS';

  $content_model_pid = 'islandora:bookCModel';

  // TODO add book collection pid to 
  $collection_pid = isset($data['collection_pid']) ?
    $data['collection_pid'] :
    variable_get('bc_islandora_yearbook_pid', 'islandora:bookCollection');

  if (!isset($data['xml'])) {
    $date = $data[3];
    $vol = explode(' ', $data[1]);
    $id_date = $vol[1];
    // TODO should work for both books and newspapers.
    $title = $data[0] . ' ' . $id_date;
    //$date_short = date('Ymd', strtotime($date));
    $extent = $data[12] . ' pages';
    $description = $data[7];

    $xml = <<<BOOK_MODS
      <mods xmlns="http://www.loc.gov/mods/v3" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.loc.gov/mods/v3 http://www.loc.gov/mods/v3/mods-3-5.xsd">
        <titleInfo>
          <title>$title</title>
        </titleInfo>

        <name type="corporate" authority="naf" authorityURI="http://id.loc.gov/authorities/names" valueURI="http://id.loc.gov/authorities/names/n79065105">
          <namePart>Barnard College</namePart>
          <role>
            <roleTerm type="text" authority="marcrelator">Publisher</roleTerm>
          </role>
        </name>

        <typeOfResource>text</typeOfResource>

        <genre authority="marcgt">yearbook</genre>
  
        <originInfo eventType="publication">
          <place>
            <placeTerm type="code" authority="marccountry">nyu</placeTerm>
            <placeTerm type="text">New York (N.Y.)</placeTerm>
          </place>
          <publisher>Barnard College</publisher>
          <dateIssued encoding="iso8601" keyDate="yes">$date</dateIssued>
          <dateCreated encoding="iso8601" keyDate="yes">$date</dateCreated>
          <issuance>continuing</issuance>
        </originInfo>

        <language>
          <languageTerm type="code" authority="iso639-2b">eng</languageTerm>
        </language>

        <physicalDescription>
          <form authority="marcform">print</form>
          <form authority="marcform">electronic</form>
          <extent>$extent</extent>
          <internetMediaType>image/jpeg2000</internetMediaType>
          <digitalOrigin>reformatted digital</digitalOrigin>
        </physicalDescription>

        <abstract>$description</abstract>

        <subject authority="lcsh" authorityURI="http://id.loc.gov/authorities/subjects" valueURI="http://id.loc.gov/authorities/subjects/sh85147738">
          <topic>Women's colleges</topic>
        </subject>
        <subject authority="lcsh" authorityURI="http://id.loc.gov/authorities/subjects" valueURI="http://id.loc.gov/authorities/subjects/sh85147501">
          <topic>Women college students</topic>
        </subject>

        <subject authority="lcsh" authorityURI="http://id.loc.gov/authorities/subjects" valueURI="http://id.loc.gov/authorities/subjects/sh85129244">
          <topic>Student activities</topic>
        </subject>

        <subject>
          <name type="corporate" authority="naf" authorityURI="http://id.loc.gov/authorities/names" valueURI="http://id.loc.gov/authorities/names/n79065105">
          </name>
        </subject>

        <relatedItem type="host"> 
          <titleInfo> 
            <title>Mortarboard</title> 
          </titleInfo> 
          <typeOfResource>text</typeOfResource> 
          <identifier type="local">BC12-01</identifier> 
        </relatedItem> 

        <identifier type="local">BC12-01_$id_date</identifier>

        <location> 
          <physicalLocation authority="marcorg">NNC</physicalLocation> 
          <physicalLocation authority="marcorg">NyNyBAW</physicalLocation> 
          <physicalLocation>Barnard College Library (New York, N.Y.)</physicalLocation> 
        </location> 

        <accessCondition type="useAndReproduction">This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License. Contact the Archives at archives@barnard.edu for more information on access and use, including commercial uses.</accessCondition>

        <recordInfo>
          <recordContentSource>Barnard Archives and Special Collections</recordContentSource>
          <recordContentSource authority="marcorg">NNC</recordContentSource>
          <recordContentSource authority="marcorg">NyNyBAW</recordContentSource>
          <languageOfCataloging>
            <languageTerm authority="iso639-2b">eng</languageTerm>
          </languageOfCataloging>
        </recordInfo>
      </mods>
BOOK_MODS;
  $rdf_string = <<<BOOK_RDF
    <rdf:RDF xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:islandora="http://islandora.ca/ontology/relsext#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
      <rdf:Description rdf:about="info:fedora/$pid">
        <fedora-model:hasModel rdf:resource="info:fedora/$content_model_pid"></fedora-model:hasModel>
        <fedora:isMemberOf rdf:resource="info:fedora/$collection_pid"></fedora:isMemberOf>
        <islandora:isSequenceNumber>1</islandora:isSequenceNumber>
        <islandora:dateIssued>$date</islandora:dateIssued>
      </rdf:Description>
    </rdf:RDF>
BOOK_RDF;
  }
  else {
    $xml = $data['xml'];
    $date = $data['date'];
    $title = $data['title'];
    $rdf_string = <<<BOOK_RDF
      <rdf:RDF xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:islandora="http://islandora.ca/ontology/relsext#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
        <rdf:Description rdf:about="info:fedora/$pid">
          <fedora-model:hasModel rdf:resource="info:fedora/$content_model_pid"></fedora-model:hasModel>
          <fedora:isMemberOf rdf:resource="info:fedora/$collection_pid"></fedora:isMemberOf>
        </rdf:Description>
      </rdf:RDF>
BOOK_RDF;
  }
  
 
  $mods_xml = new DOMDocument();
  $mods_xml->loadXML($xml);

  $transform_xsl = new DOMDocument();
  $xsl_file = DRUPAL_ROOT . '/' . drupal_get_path('module', 'islandora_importer') . '/xsl/mods_to_dc.xsl';
  $transform_xsl->load($xsl_file);

  $proc = new XSLTProcessor();
  $proc->importStylesheet($transform_xsl);
  $dc = $proc->transformToXML($mods_xml);

  $object = new NewFedoraObject($pid, $connection->repository);
  
  $object->label = $title; 
 
  $dc_datastream = new NewFedoraDatastream('DC', 'X', $object, $connection->repository);

  $dc_datastream->content = $dc;
  $dc_datastream->mimetype = 'text/xml';
  $dc_datastream->label = 'Dublin Core Record for this object';

  $object->ingestDatastream($dc_datastream);

  $mods_datastream = new NewFedoraDatastream('MODS', 'X', $object, $connection->repository);
  
  $mods_datastream->setContentFromString($xml);
  $mods_datastream->mimetype = 'text/xml';
  $mods_datastream->label = 'MODS record';
  $mods_datastream->checksum = TRUE;
  $mods_datastream->checksumType = 'MD5';
  $mods_datastream->logMessage = 'MODS datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($mods_datastream);
  
  $rels_datastream = new NewFedoraDatastream('RELS-EXT', 'X', $object, $connection->repository);
  $rels_datastream->setContentFromString($rdf_string);
  $rels_datastream->label = 'RELS-EXT';
  $rels_datastream->mimetype = 'text/xml';
  $rels_datastream->logMessage = 'RELS-EXT datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($rels_datastream);  
  try {
    $object->state = 'A';
    $ingest = $tuque->repository->ingestObject($object);
  }
  catch(Exception $e) {
    unset($object);
    throw($e);
  }

  if ($ingest == TRUE) {
    drush_log(dt("Book object @pid created successfully\n", array('@pid' => $pid)), 'success');
  }
  else {
    drush_log(dt("There was an error creating the book object!\n", 'warning'));
  }
  unset($object);
  unset($mods_datastream);
  unset($rels_datastream);
  unset($dc_datastream);
  
  return $pid;
}

function create_pub_object($pid, $data) {
  module_load_include('inc', 'islandora', 'RestConnection');
  
  $tuque = islandora_get_tuque_connection();
  $connection = $tuque->connection;
  
  $dsid = 'MODS';

  $content_model_pid = 'islandora:collectionCModel';

  // TODO add book collection pid to 
  $collection_pid = variable_get('bc_islandora_pubs_pid', 'islandora:1022');

  $xml = $data['xml'];
  
  $rdf_string = <<<PUB_RDF
    <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:islandora="http://islandora.ca/ontology/relsext#">
      <rdf:Description rdf:about="info:fedora/$pid">
        <fedora:isMemberOf rdf:resource="info:fedora/$collection_pid"></fedora:isMemberOf>
        <fedora-model:hasModel rdf:resource="info:fedora/$content_model_pid"></fedora-model:hasModel>
      </rdf:Description>
    </rdf:RDF>
PUB_RDF;
 
  $mods_xml = new DOMDocument();
  $mods_xml->loadXML($xml);

  $transform_xsl = new DOMDocument();
  $xsl_file = DRUPAL_ROOT . '/' . drupal_get_path('module', 'islandora_importer') . '/xsl/mods_to_dc.xsl';
  $transform_xsl->load($xsl_file);

  $proc = new XSLTProcessor();
  $proc->importStylesheet($transform_xsl);
  $dc = $proc->transformToXML($mods_xml);

  $object = new NewFedoraObject($pid, $connection->repository);
  
  $object->label = $data['title']; 
 
  $dc_datastream = new NewFedoraDatastream('DC', 'X', $object, $connection->repository);

  $dc_datastream->content = $dc;
  $dc_datastream->mimetype = 'text/xml';
  $dc_datastream->label = 'Dublin Core Record for this object';

  $object->ingestDatastream($dc_datastream);

  $mods_datastream = new NewFedoraDatastream('MODS', 'X', $object, $connection->repository);
  
  $mods_datastream->setContentFromString($xml);
  $mods_datastream->mimetype = 'text/xml';
  $mods_datastream->label = 'MODS record';
  $mods_datastream->checksum = TRUE;
  $mods_datastream->checksumType = 'MD5';
  $mods_datastream->logMessage = 'MODS datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($mods_datastream);
  
  $rels_datastream = new NewFedoraDatastream('RELS-EXT', 'X', $object, $connection->repository);
  $rels_datastream->setContentFromString($rdf_string);
  $rels_datastream->label = 'RELS-EXT';
  $rels_datastream->mimetype = 'text/xml';
  $rels_datastream->logMessage = 'RELS-EXT datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($rels_datastream);  
  try {
    $object->state = 'A';
    $ingest = $tuque->repository->ingestObject($object);
  }
  catch(Exception $e) {
    unset($object);
    throw($e);
  }

  if ($ingest == TRUE) {
    drush_log(dt("Publication collection object @pid created successfully\n", array('@pid' => $pid)), 'success');
  }
  else {
    drush_log(dt("There was an error creating the book object!\n", 'warning'));
  }
  unset($object);
  unset($mods_datastream);
  unset($rels_datastream);
  unset($dc_datastream);
  
  return $pid;
}

function create_page_object($file_path, $data, $collection_pid, $page_pid, $type) {
  $connection = islandora_get_tuque_connection();
  
  $pid = $page_pid;

  switch ($type) {
    case 'newspaper':
      $date = $data[1];
      $date_short = date('Ymd', strtotime($date));    
      $page_number = $data[3];
      $title = 'Barnard Bulletin';
      $label = $title . ', ' . $date . ', page ' . $page_number;
      $dateIssued = '<dateIssued encoding="w3cdtf">' . date('Y-m-d', strtotime($date)) . '</dateIssued>';
      $dateCreated = '<dateCreated encoding="w3cdtf">' . date('Y-m-d', strtotime($date)) . '</dateCreated>';
      $id_short = 'BC12-03';
      $id_long = 'BC12-03_' . $date_short . '-' . $page_number;
      $file_name = $file_path . '/' . $data[2];
      $content_model_pid = 'islandora:newspaperPageCModel';
      $genre = 'newspaper';
      $img_mime = 'image/tiff';
    break;
    case 'book':
      $date = $data[1];
      $date_short = date('Ymd', strtotime($date));    
      $title = $data[0];
      $year = array_pop(explode(' ', $data[1]));
      $page_number = array_pop($data);
      //$label = $title . ' ' . $data[3] . ', page ' . $page_number;
      $label = $title . ' ' . $year . ', page ' . $page_number; 
      $dateIssued = '<dateIssued encoding="iso8601">' . $data[3] . '</dateIssued>';
      $dateCreated = '<dateCreated encoding="iso8601">' . $data[3] . '</dateCreated>';
      $id_short = 'BC12-01';
      $id_long = 'BC12-01_' . $data[3] . '_' . str_pad($page_number, 3, '0', STR_PAD_LEFT);
      $file_name = $file_path;
      $content_model_pid = 'islandora:pageCModel';
      $genre = 'yearbook';
      $img_mime = 'image/jp2';
    break;
    case 'pub':
      $content_model_pid = 'islandora:pageCModel';
      $page_number = $data['page_number'];
      $img_mime = 'image/tiff';
      $file_name = $file_path;
      $label = $data['title'];

      $title = $label;
      $genre = $data['genre'];
      $title_related = $data['title_related'];
      $title_host = $data['title_host'];
      $host_id = $data['host_id'];
      $date = $data['date'];
      $page_id = $data['page_id'];
      $xml = <<<PAGE_MODS
        <mods xmlns="http://www.loc.gov/mods/v3" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <titleInfo>
            <title>$title</title>
          </titleInfo>
          <genre authority="lcgft" authorityURI="http://id.loc.gov/authorities/genreForms" valueURI="http://id.loc.gov/authorities/genreForms/gf2014026139">$genre</genre>
          <relatedItem type="host" displayLabel="Record Group">
            <titleInfo>
              <title>$title_related</title>
            </titleInfo>
          </relatedItem>
          <relatedItem type="host">
            <titleInfo>
              <title>$title_host</title>
            </titleInfo>
            <typeOfResource>text</typeOfResource>
            <identifier type="local">$host_id</identifier>
          </relatedItem>
          <identifier type="local">$page_id</identifier>
          <originInfo>
            <dateCreated encoding="w3cdtf">$date</dateCreated>
          </originInfo>
        </mods>
PAGE_MODS;
    break;
  }

  if ($page_number == 1) {
    $rand = rand('100', '999');
    $tmp_file = '/tmp/tmp_file' . $rand . '.jpg';
    $command = 'convert -quiet -thumbnail 200x200 ' . $file_name . ' ' . $tmp_file;
    exec($command, $exec_output, $exec_return);
    if ($exec_return == 0) {
      $book_item = new FedoraObject($collection_pid, $connection->repository);
      $tn_datastream = new NewFedoraDatastream('TN', 'M', $book_item, $connection->repository);
      $tn_datastream->setContentFromFile($tmp_file, FALSE);
      $tn_datastream->label = 'Thumbnail';
      $tn_datastream->mimetype = 'image/jpg';
      $tn_datastream->checksum = TRUE;
      $tn_datastream->checksumType = 'MD5';
      $tn_datastream->logMessage = "TN derivative created by ImageMagick with command - $command || SUCCESS";
      $book_item->ingestDatastream($tn_datastream);
      unlink($tmp_file);
    }
  }

  $dsid = 'MODS';

  if (!isset($xml)) {
    $xml = <<<PAGE_MODS
      <mods xmlns="http://www.loc.gov/mods/v3" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
        <titleInfo>
          <title>$label</title>
        </titleInfo>
        <genre authority="marcgt">$genre</genre>
        <originInfo>
          $dateIssued
          $dateCreated
        </originInfo>
        <relatedItem type="host"> 
          <titleInfo> 
            <title>$title</title> 
          </titleInfo> 
          <typeOfResource>text</typeOfResource> 
          <identifier type="local">$id_short</identifier> 
        </relatedItem> 
        <identifier type="local">$id_long</identifier>
      </mods>
PAGE_MODS;
  }
  
  $rdf_string = <<<PAGE_RDF
    <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:islandora="http://islandora.ca/ontology/relsext#">
      <rdf:Description rdf:about="info:fedora/$pid">
        <fedora-model:hasModel rdf:resource="info:fedora/$content_model_pid"></fedora-model:hasModel>
        <islandora:hasLanguage></islandora:hasLanguage>
        <islandora:isPageOf rdf:resource="info:fedora/$collection_pid"></islandora:isPageOf>
        <islandora:isSequenceNumber>$page_number</islandora:isSequenceNumber>
        <islandora:isPageNumber>$page_number</islandora:isPageNumber>
        <islandora:isSection>1</islandora:isSection>
        <fedora:isMemberOf rdf:resource="info:fedora/$collection_pid"></fedora:isMemberOf>
      </rdf:Description>
    </rdf:RDF>
PAGE_RDF;
  print "$xml\n";
  $mods_xml = new DOMDocument();
  $mods_xml->loadXML($xml);

  $transform_xsl = new DOMDocument();
  $xsl_file = DRUPAL_ROOT . '/' . drupal_get_path('module', 'islandora_importer') . '/xsl/mods_to_dc.xsl';
  $transform_xsl->load($xsl_file);

  $proc = new XSLTProcessor();
  $proc->importStylesheet($transform_xsl);
  $dc = $proc->transformToXML($mods_xml);

  $object = new NewFedoraObject($pid, $connection->repository);

  $object->label = $label;

  $dc_datastream = new NewFedoraDatastream('DC', 'X', $object, $connection->repository);

  $dc_datastream->content = $dc;
  $dc_datastream->mimetype = 'text/xml';
  $dc_datastream->label = 'Dublin Core Record for this object';

  $object->ingestDatastream($dc_datastream);

  $mods_datastream = new NewFedoraDatastream('MODS', 'X', $object, $connection->repository);

  $mods_datastream->content = $xml;
  $mods_datastream->mimetype = 'text/xml';
  $mods_datastream->label = 'MODS record';
  $mods_datastream->checksum = TRUE;
  $mods_datastream->checksumType = 'MD5';
  $mods_datastream->logMessage = 'MODS datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($mods_datastream);
  
  $image_datastream = new NewFedoraDatastream('OBJ', 'M', $object, $connection->repository);
  $image_datastream->setContentFromFile($file_name, FALSE);
  // $image_datastream->label = 'TIFF image';
  $image_datastream->label = $label;
  // $image_datastream->mimetype = 'image/tiff';
  $image_datastream->mimetype = $img_mime;
  $image_datastream->checksum = TRUE;
  $image_datastream->checksumType = 'MD5';
  $image_datastream->logMessage = 'OBJ datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($image_datastream);

  $rels_datastream = new NewFedoraDatastream('RELS-EXT', 'X', $object, $connection->repository);
  $rels_datastream->setContentFromString($rdf_string);
  $rels_datastream->label = 'Fedora Object to Object Relationship Metadata.';
  $rels_datastream->mimetype = 'text/xml';
  $rels_datastream->logMessage = 'RELS-EXT datastream created using Newspapers batch ingest script || SUCCESS';

  $object->ingestDatastream($rels_datastream);
  try {
    $object->state = 'A';
    $ingest = $connection->repository->ingestObject($object);
  }
  catch (Exception $e) {
    unset($object);
    throw($e);
  }

  if ($ingest == TRUE) {
    drush_log(dt("Page object @pid created successfully\n", array('@pid' => $pid)), 'success');
  }
  else {
    drush_log(dt("There was an error creating the page object!\n"), 'error');
  }

  unset($object); // make sure destructors are called as this script can run for a long time.
  unset($image_datastream);
  unset($mods_datastream);
  unset($rels_datastream);
  unset($dc_datastream);

  return $pid;
}
