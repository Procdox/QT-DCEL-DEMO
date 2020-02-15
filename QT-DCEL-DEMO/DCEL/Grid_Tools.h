#pragma once
#include "Grid_Region.h"

//==========================================================================================================
//========================================== transforms ====================================================
//==========================================================================================================

void mergeGroup(std::list<Region *> & nulls);

///<summary>
///<para>Trims small sub-regions from target with Cull and merges them to outs</para>
///<para>&#160;</para>
///<para>Assumes: -</para>
///<para>Fulfills: outs is maximally merged. target is maximally merged</para>
///</summary>
void removeSmallSections(std::list<Region *> &target, grd const &min_width, std::list<Region *> &smalls);

///<summary>
///<para>Trims small sub-regions from target with Cull and merges them to outs</para>
///<para>&#160;</para>
///<para>Assumes: target is maximally merged</para>
///<para>Fulfills: outs is maximally merged. target is simple, target are otherwise restricted to width</para>
///</summary>
void sizeRestrict(std::list<Region *> &target, grd const &min_width, std::list<Region *> &outs);

///<summary>
///<para>Allocates a novel set of sub-regions, from a set of regions, as defined by a boundary.</para>
///<para>&#160;</para>
///<para>Assumes: -</para>
///<para>Fulfills: set is maximally merged. result is maximally merged</para>
///</summary>
std::list<Region *> allocateBoundaryFrom(std::list<Pgrd> const &boundary, std::list<Region *> &set);

///<summary>
///<para>Allocates a novel set of sub-regions, from a set of regions, as defined by a boundary.</para>
///<para>&#160;</para>
///<para>Assumes: -</para>
///<para>Fulfills: set is maximally merged. result is maximally merged</para>
///</summary>
void allocateBoundaryFromInto(std::list<Pgrd> const &boundary, std::list<Region *> &set, std::list<Region *> &result);

///<summary>
///<para>Allocates a novel set of sub-regions, from a set of regions, as defined by a boundary. Then trims small sub-regions with Cull and returns them to the originating set of regions.</para>
///<para>&#160;</para>
///<para>Assumes: -</para>
///<para>Fulfills: set is maximally merged. result is simple, merges are otherwise restricted to width</para>
///</summary>
std::list<Region *> allocateCleanedBoundaryFrom(std::list<Pgrd> const &boundary, grd const &min_width, std::list<Region *> &set);

///<summary>
///<para>Allocates a novel set of sub-regions, from a set of regions, as defined by a boundary. Then trims small sub-regions with Cull and returns them to the originating set of regions.</para>
///<para>&#160;</para>
///<para>Assumes: -</para>
///<para>Fulfills: set is maximally merged. result is simple, merges are otherwise restricted to width</para>
///</summary>
void allocateCleanedBoundaryFromInto(std::list<Pgrd> const &boundary, grd const &min_width, std::list<Region *> &set, std::list<Region *> &result);