-- phpMyAdmin SQL Dump
-- version 4.7.0
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Gegenereerd op: 08 okt 2017 om 00:36
-- Serverversie: 10.1.25-MariaDB
-- PHP-versie: 5.6.31

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `lwo`
--

-- --------------------------------------------------------

--
-- Tabelstructuur voor tabel `accounts`
--

CREATE TABLE `accounts` (
  `id` bigint(64) NOT NULL,
  `username` text NOT NULL,
  `password` text NOT NULL,
  `email` text NOT NULL,
  `bBanned` tinyint(1) NOT NULL,
  `gmLevel` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabelstructuur voor tabel `items`
--

CREATE TABLE `items` (
  `objectID` bigint(64) NOT NULL,
  `ownerID` bigint(64) NOT NULL,
  `LOT` bigint(64) NOT NULL,
  `bEquipped` tinyint(1) NOT NULL,
  `equipLocation` text NOT NULL,
  `slot` int(10) NOT NULL,
  `bagID` int(10) NOT NULL,
  `count` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabelstructuur voor tabel `minifigs`
--

CREATE TABLE `minifigs` (
  `objectID` bigint(64) NOT NULL,
  `accountID` bigint(64) NOT NULL,
  `playerName` text NOT NULL,
  `tempName` text NOT NULL,
  `bNameApproved` tinyint(1) NOT NULL,
  `gmlevel` int(9) NOT NULL,
  `eyes` int(10) NOT NULL,
  `eyeBrows` int(10) NOT NULL,
  `mouth` int(10) NOT NULL,
  `hair` int(10) NOT NULL,
  `hairColor` int(10) NOT NULL,
  `health` int(10) NOT NULL,
  `armor` int(10) NOT NULL,
  `imagination` int(10) NOT NULL,
  `maxHealth` int(10) NOT NULL,
  `maxArmor` int(10) NOT NULL,
  `maxImagination` int(10) NOT NULL,
  `selectedConsumable` int(10) NOT NULL,
  `lastZoneID` bigint(64) NOT NULL,
  `lh` int(10) NOT NULL,
  `rh` int(10) NOT NULL,
  `posX` int(10) DEFAULT NULL,
  `posY` int(10) NOT NULL,
  `posZ` int(10) NOT NULL,
  `rotX` float NOT NULL,
  `rotY` float NOT NULL,
  `rotZ` float NOT NULL,
  `rotW` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabelstructuur voor tabel `servers`
--

CREATE TABLE `servers` (
  `id` bigint(64) NOT NULL,
  `ip` text NOT NULL,
  `port` int(9) NOT NULL,
  `version` int(9) NOT NULL,
  `zoneID` int(9) NOT NULL,
  `cloneID` int(9) NOT NULL,
  `instanceID` int(9) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabelstructuur voor tabel `zones`
--

CREATE TABLE `zones` (
  `id` bigint(64) NOT NULL,
  `checksum` bigint(64) NOT NULL,
  `maxplayers` int(9) NOT NULL,
  `gmlevel` int(9) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Gegevens worden geëxporteerd voor tabel `zones`
--

INSERT INTO `zones` (`id`, `checksum`, `maxplayers`, `gmlevel`) VALUES
(1000, 816, 9, 0),
(1100, 3798, 15, 0),
(1200, 3999, 20, 0),
(1250, 440, 6, 0),
(1300, 5864, 20, 0);

--
-- Indexen voor geëxporteerde tabellen
--

--
-- Indexen voor tabel `accounts`
--
ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`);

--
-- Indexen voor tabel `items`
--
ALTER TABLE `items`
  ADD PRIMARY KEY (`objectID`),
  ADD KEY `objectID` (`objectID`);

--
-- Indexen voor tabel `minifigs`
--
ALTER TABLE `minifigs`
  ADD PRIMARY KEY (`objectID`);

--
-- Indexen voor tabel `servers`
--
ALTER TABLE `servers`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id` (`id`);

--
-- Indexen voor tabel `zones`
--
ALTER TABLE `zones`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT voor geëxporteerde tabellen
--

--
-- AUTO_INCREMENT voor een tabel `accounts`
--
ALTER TABLE `accounts`
  MODIFY `id` bigint(64) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT voor een tabel `items`
--
ALTER TABLE `items`
  MODIFY `objectID` bigint(64) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1152994869473527238;
--
-- AUTO_INCREMENT voor een tabel `minifigs`
--
ALTER TABLE `minifigs`
  MODIFY `objectID` bigint(64) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1152921510473527056;
--
-- AUTO_INCREMENT voor een tabel `servers`
--
ALTER TABLE `servers`
  MODIFY `id` bigint(64) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=42;
--
-- AUTO_INCREMENT voor een tabel `zones`
--
ALTER TABLE `zones`
  MODIFY `id` bigint(64) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1301;COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
