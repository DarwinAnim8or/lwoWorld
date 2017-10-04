-- phpMyAdmin SQL Dump
-- version 4.7.0
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Gegenereerd op: 03 okt 2017 om 17:19
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
(1000, 0, 9, 0),
(1100, 0, 15, 0),
(1200, 3999, 20, 0);

--
-- Indexen voor geëxporteerde tabellen
--

--
-- Indexen voor tabel `accounts`
--
ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`);

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
-- AUTO_INCREMENT voor een tabel `servers`
--
ALTER TABLE `servers`
  MODIFY `id` bigint(64) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT voor een tabel `zones`
--
ALTER TABLE `zones`
  MODIFY `id` bigint(64) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1201;COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
